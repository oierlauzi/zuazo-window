#include <zuazo/Outputs/Window.h>

#include "../GLFW.h"

#include <zuazo/Graphics/Vulkan.h>
#include <zuazo/Graphics/VulkanConversions.h>
#include <zuazo/Graphics/ColorTransfer.h>
#include <zuazo/Graphics/StagedBuffer.h>
#include <zuazo/Utils/StaticId.h>
#include <zuazo/Utils/Functions.h>
#include <zuazo/Math/Functions.h>

#include <cmath>
#include <algorithm>
#include <limits>
#include <set>
#include <bitset>
#include <mutex>


namespace Zuazo::Outputs {

/*
 * Window::Impl
 */
struct Window::Impl {
	struct Open {
		struct Vertex {
			Math::Vec2f position;
			Math::Vec2f texCoord;
		};


		enum DescriptorLayouts {
			DESCRIPTOR_LAYOUT_WINDOW,
			DESCRIPTOR_LAYOUT_FRAME,

			DESCRIPTOR_LAYOUT_COUNT
		};

		enum WindowDescriptors {
			WINDOW_DESCRIPTOR_VIEWPORT,
			WINDOW_DESCRIPTOR_COLOR_TRANSFER,

			WINDOW_DESCRIPTOR_COUNT
		};


		static constexpr uint32_t VERTEX_BUFFER_BINDING = 0;
		static constexpr uint32_t VERTEX_POSITION = 0;
		static constexpr uint32_t VERTEX_TEXCOORD = 1;

		static constexpr size_t COLOR_TRANSFER_UNIFORM_OFFSET = 0;
		static inline const size_t COLOR_TRANSFER_UNIFORM_SIZE = Graphics::ColorTransfer::size();
		static inline const size_t VIEWPORT_UNIFORM_OFFSET = Utils::align(COLOR_TRANSFER_UNIFORM_OFFSET + COLOR_TRANSFER_UNIFORM_SIZE, 0x100); //256 is the maximum
		static constexpr size_t VIEWPORT_UNIFORM_SIZE = sizeof(glm::vec2);

		static inline const size_t UNIFORM_BUFFER_SIZE = VIEWPORT_UNIFORM_OFFSET + VIEWPORT_UNIFORM_SIZE;



		const Graphics::Vulkan&						vulkan;

		GLFW::Window								window;
		vk::UniqueSurfaceKHR						surface;
		vk::UniqueCommandPool						commandPool;
		vk::CommandBuffer							commandBuffer;
		Graphics::StagedBuffer						vertexBuffer;
		Graphics::StagedBuffer						uniformBuffer;
		vk::UniqueDescriptorPool					descriptorPool;
		vk::DescriptorSet							descriptorSet;
		vk::UniqueSemaphore 						imageAvailableSemaphore;
		vk::UniqueSemaphore							renderFinishedSemaphore;
		vk::UniqueFence								renderFinishedFence;

		vk::Extent2D								extent;
		vk::Format 									format;
		vk::ColorSpaceKHR 							colorSpace;
		Graphics::ColorTransfer						colorTransfer;
		vk::Filter									filter;
		Graphics::Frame::Geometry					geometry;

		vk::UniqueSwapchainKHR						swapchain;
		std::vector<vk::UniqueImageView>			swapchainImageViews;
		vk::UniqueRenderPass						renderPass;
		std::vector<vk::UniqueFramebuffer>			framebuffers;
		vk::PipelineLayout							pipelineLayout;
		vk::UniquePipeline							pipeline;	


		Open(	const Graphics::Vulkan& vulkan,
				Math::Vec2i size,
				std::string_view name,
				GLFW::Window::Callbacks cbks,
				vk::Format format,
				vk::ColorSpaceKHR colorSpace,
				Graphics::ColorTransfer colorTransfer,
				ScalingMode scalingMode,
				ScalingFilter scalingFilter ) 
			: vulkan(vulkan)
			, window(size, name, std::move(cbks))
			, surface(window.getSurface(vulkan))
			, commandPool(createCommandPool(vulkan))
			, commandBuffer(createCommandBuffer(vulkan, *commandPool))
			, vertexBuffer(createVertexBuffer(vulkan))
			, uniformBuffer(createUniformBuffer(vulkan))
			, descriptorPool(createDescriptorPool(vulkan))
			, descriptorSet(createDescriptorSet(vulkan, *descriptorPool))
			, imageAvailableSemaphore(vulkan.createSemaphore())
			, renderFinishedSemaphore(vulkan.createSemaphore())
			, renderFinishedFence(vulkan.createFence(true))

			, extent(Graphics::toVulkan(window.getResolution()))
			, format(format)
			, colorSpace(colorSpace)
			, colorTransfer(std::move(colorTransfer))
			, filter(Graphics::toVulkan(scalingFilter))
			, geometry(createGeometry(vertexBuffer.data(), scalingMode, Math::Vec2f(extent.width, extent.height)))

			, swapchain(createSwapchain(vulkan, *surface, extent, format, colorSpace, {}))
			, swapchainImageViews(createImageViews(vulkan, *swapchain, format))
			, renderPass(createRenderPass(vulkan, format))
			, framebuffers(createFramebuffers(vulkan, *renderPass, swapchainImageViews, extent))
			, pipelineLayout(createPipelineLayout(vulkan, filter))
			, pipeline(createPipeline(vulkan, *renderPass, pipelineLayout, extent))

		{
			writeDescriptorSets();
			updateUniforms();
		}

		~Open() {
			vertexBuffer.waitCompletion(vulkan);
			uniformBuffer.waitCompletion(vulkan);
			waitCompletion();
		}

		void reconfigure(vk::Format fmt, vk::ColorSpaceKHR cs, Graphics::ColorTransfer ct) {
			enum {
				RECREATE_SWAPCHAIN,
				UPDATE_COLOR_TRANSFER,

				MODIFICATION_COUNT
			};

			std::bitset<MODIFICATION_COUNT> modifications;

			if(format != fmt) {
				//Format has changed
				format = fmt;

				modifications.set(RECREATE_SWAPCHAIN);
			}

			if(colorSpace != cs) {
				//Color space has changed
				colorSpace = cs;

				modifications.set(RECREATE_SWAPCHAIN);
			}

			if(colorTransfer != ct) {
				//Color transfer characteristics have changed
				colorTransfer = std::move(ct);

				modifications.set(UPDATE_COLOR_TRANSFER);
			}

			//Recreate stuff accordingly
			if(modifications.any()) {
				//Wait until rendering finishes
				waitCompletion();

				if(modifications.test(RECREATE_SWAPCHAIN)) recreateSwapchain();
				if(modifications.test(UPDATE_COLOR_TRANSFER)) updateColorTransferUniform();
			}
		}

		void setScalingMode(ScalingMode mode) {
			waitCompletion();
			geometry.setScalingMode(mode);
		}

		void setScalingFilter(ScalingFilter filt) {
			waitCompletion();
			filter = Graphics::toVulkan(filt);
			recreatePipelineLayout();
		}

		void draw(const std::shared_ptr<const Graphics::Frame>& frame) {
			//Wait for the previous rendering to be completed
			waitCompletion();

			//Acquire an image from the swapchain
			const auto index = acquireImage();

			//Resize the geometry if needed
			if(frame) {
				vertexBuffer.waitCompletion(vulkan);
				if(geometry.useFrame(*frame)){
					//Vertex buffer has changed, update it
					vertexBuffer.flushData(
						vulkan, 
						vulkan.getGraphicsQueueIndex(),
						vk::AccessFlagBits::eVertexAttributeRead,
						vk::PipelineStageFlagBits::eVertexInput
					);
				}	
			}

			const auto& frameBuffer = *(framebuffers[index]);

			//Begin writing to the command buffer. //TODO maybe reset pool?
			constexpr vk::CommandBufferBeginInfo cmdBegin(
				vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 
				nullptr
			);
			vulkan.begin(commandBuffer, cmdBegin);

			//Begin a render pass
			const std::array clearValue = {
				vk::ClearValue(std::array{ 0.0f, 0.0f, 0.0f, 0.0f })
			};
			const vk::RenderPassBeginInfo rendBegin(
				*renderPass,														//Renderpass
				frameBuffer,														//Target framebuffer
				vk::Rect2D({0, 0}, extent),											//Extent
				clearValue.size(), clearValue.data()								//Attachment clear values
			);
			vulkan.beginRenderPass(commandBuffer, rendBegin, vk::SubpassContents::eInline);


			//If it is a valid frame, draw it.
			if(frame) {
				vulkan.bindPipeline(commandBuffer, vk::PipelineBindPoint::eGraphics, *pipeline);

				vulkan.bindVertexBuffers(
					commandBuffer,													//Command buffer
					VERTEX_BUFFER_BINDING,											//Binding
					vertexBuffer.getBuffer(),										//Vertex buffers
					0UL																//Offsets
				);

				vulkan.bindDescriptorSets(
					commandBuffer,													//Command buffer
					vk::PipelineBindPoint::eGraphics,								//Pipeline bind point
					pipelineLayout,													//Pipeline layout
					DESCRIPTOR_LAYOUT_WINDOW,										//First index
					descriptorSet,													//Descriptor sets
					{}																//Dynamic offsets
				);

				frame->bind(commandBuffer, pipelineLayout, DESCRIPTOR_LAYOUT_FRAME, filter);

				vulkan.draw(commandBuffer, Graphics::Frame::Geometry::VERTEX_COUNT, 1, 0, 0);
			}

			//End everything
			vulkan.endRenderPass(commandBuffer);
			vulkan.end(commandBuffer);

			//Send it to the queue
			const std::array imageAvailableSemaphores = {
				*imageAvailableSemaphore
			};
			const std::array renderFinishedSemaphores = {
				*renderFinishedSemaphore
			};
			const std::array commandBuffers = {
				commandBuffer
			};
			const std::array pipelineStages = {
				vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			};
			const vk::SubmitInfo subInfo(
				imageAvailableSemaphores.size(), imageAvailableSemaphores.data(),	//Wait semaphores
				pipelineStages.data(),												//Pipeline stages
				commandBuffers.size(), commandBuffers.data(),						//Command buffers
				renderFinishedSemaphores.size(), renderFinishedSemaphores.data()	//Signal semaphores
			);
			vulkan.resetFences(*renderFinishedFence);
			vulkan.submit(vulkan.getGraphicsQueue(), subInfo, *renderFinishedFence);

			//Present it
			vulkan.present(*swapchain, index, renderFinishedSemaphores.front());
		}

		void resizeFramebuffer(Resolution res) {
			waitCompletion();

			extent = Graphics::toVulkan(res);
			geometry.setTargetSize(res);
			updateViewportUniform();
			recreateSwapchain();
		}


	private:
		void recreateSwapchain() {
			swapchain = createSwapchain(vulkan, *surface, extent, format, colorSpace, *swapchain);
			swapchainImageViews = createImageViews(vulkan, *swapchain, format);
			renderPass = createRenderPass(vulkan, format);
			framebuffers = createFramebuffers(vulkan, *renderPass, swapchainImageViews, extent);
			pipeline = createPipeline(vulkan, *renderPass, pipelineLayout, extent);
		}

		void recreatePipelineLayout() {
			pipelineLayout = createPipelineLayout(vulkan, filter);
			pipeline = createPipeline(vulkan, *renderPass, pipelineLayout, extent);
		}

		void updateViewportUniform() {
			uniformBuffer.waitCompletion(vulkan);

			auto& size = *(reinterpret_cast<Math::Vec2f*>(uniformBuffer.data() + VIEWPORT_UNIFORM_OFFSET));
			size = geometry.getTargetSize();

			uniformBuffer.flushData(
				vulkan,
				VIEWPORT_UNIFORM_OFFSET,
				VIEWPORT_UNIFORM_SIZE,
				vulkan.getGraphicsQueueIndex(),
				vk::AccessFlagBits::eUniformRead,
				vk::PipelineStageFlagBits::eVertexShader
			);
		}

		void updateColorTransferUniform() {
			uniformBuffer.waitCompletion(vulkan);
			
			std::memcpy(
				uniformBuffer.data() + COLOR_TRANSFER_UNIFORM_OFFSET,
				colorTransfer.data(),
				COLOR_TRANSFER_UNIFORM_SIZE
			);

			uniformBuffer.flushData(
				vulkan,
				COLOR_TRANSFER_UNIFORM_OFFSET,
				COLOR_TRANSFER_UNIFORM_SIZE,
				vulkan.getGraphicsQueueIndex(),
				vk::AccessFlagBits::eUniformRead,
				vk::PipelineStageFlagBits::eFragmentShader
			);
		}

		void updateUniforms() {
			uniformBuffer.waitCompletion(vulkan);		
			
			auto& size = *(reinterpret_cast<Math::Vec2f*>(uniformBuffer.data() + VIEWPORT_UNIFORM_OFFSET));
			size = geometry.getTargetSize();

			std::memcpy(
				uniformBuffer.data() + COLOR_TRANSFER_UNIFORM_OFFSET,
				colorTransfer.data(),
				COLOR_TRANSFER_UNIFORM_SIZE
			);

			uniformBuffer.flushData(
				vulkan,
				vulkan.getGraphicsQueueIndex(),
				vk::AccessFlagBits::eUniformRead,
				vk::PipelineStageFlagBits::eVertexShader |
				vk::PipelineStageFlagBits::eFragmentShader
			);
		}

		void waitCompletion() {
			vulkan.waitForFences(*renderFinishedFence);
		}

		uint32_t acquireImage() {
			vk::Result result;
			uint32_t index;

			//Try to acquire an image as many times as needed.
			do {
				result = vulkan.getDevice().acquireNextImageKHR(
					*swapchain, 						
					Graphics::Vulkan::NO_TIMEOUT,
					*imageAvailableSemaphore,
					nullptr,
					&index,
					vulkan.getDispatcher()
				);

				//Evaluate wether it was a success
				switch(result) {
				case vk::Result::eErrorOutOfDateKHR:
				case vk::Result::eSuboptimalKHR:
					recreateSwapchain();
					break;

				default: 
					break;
				}
			} while(result != vk::Result::eSuccess);

			return index;
		}
	
		void writeDescriptorSets() {
			const std::array viewportBuffers = {
				vk::DescriptorBufferInfo(
					uniformBuffer.getBuffer(),								//Buffer
					VIEWPORT_UNIFORM_OFFSET,								//Offset
					VIEWPORT_UNIFORM_SIZE									//Size
				)
			};
			const std::array colorTransferBuffers = {
				vk::DescriptorBufferInfo(
					uniformBuffer.getBuffer(),								//Buffer
					COLOR_TRANSFER_UNIFORM_OFFSET,							//Offset
					COLOR_TRANSFER_UNIFORM_SIZE								//Size
				)
			};

			const std::array writeDescriptorSets = {
				vk::WriteDescriptorSet( //Viewport UBO
					descriptorSet,											//Descriptor set
					WINDOW_DESCRIPTOR_VIEWPORT,								//Binding
					0, 														//Index
					viewportBuffers.size(),									//Descriptor count		
					vk::DescriptorType::eUniformBuffer,						//Descriptor type
					nullptr, 												//Images 
					viewportBuffers.data(), 								//Buffers
					nullptr													//Texel buffers
				),
				vk::WriteDescriptorSet( //ColorTransfer UBO
					descriptorSet,											//Descriptor set
					WINDOW_DESCRIPTOR_COLOR_TRANSFER,						//Binding
					0, 														//Index
					colorTransferBuffers.size(),							//Descriptor count		
					vk::DescriptorType::eUniformBuffer,						//Descriptor type
					nullptr, 												//Images 
					colorTransferBuffers.data(), 							//Buffers
					nullptr													//Texel buffers
				)
			};

			vulkan.updateDescriptorSets(writeDescriptorSets, {});
		}



		static vk::UniqueCommandPool createCommandPool(const Graphics::Vulkan& vulkan)
		{
			constexpr auto createFlags = 
				vk::CommandPoolCreateFlagBits::eTransient | 		//Re-recorded often
				vk::CommandPoolCreateFlagBits::eResetCommandBuffer;	//Re-recorded individually

			const vk::CommandPoolCreateInfo createInfo(
				createFlags,										//Flags
				vulkan.getGraphicsQueueIndex()						//Queue index
			);

			return vulkan.createCommandPool(createInfo);
		}

		static vk::CommandBuffer createCommandBuffer(	const Graphics::Vulkan& vulkan,
														vk::CommandPool pool )
		{
			const vk::CommandBufferAllocateInfo allocInfo(
				pool,
				vk::CommandBufferLevel::ePrimary,
				1
			);

			//Perform a manual allocation, as there is only need for one. 
			//Dont use a smart handle, as it gets freed automatically with the command pool
			vk::CommandBuffer result;
			vulkan.getDevice().allocateCommandBuffers(&allocInfo, &result, vulkan.getDispatcher());
			return result;
		}


		static Graphics::StagedBuffer createVertexBuffer(const Graphics::Vulkan& vulkan) {
			return Graphics::StagedBuffer(
				vulkan,
				vk::BufferUsageFlagBits::eVertexBuffer,
				sizeof(Vertex) * Graphics::Frame::Geometry::VERTEX_COUNT
			);
		}

		static Graphics::StagedBuffer createUniformBuffer(const Graphics::Vulkan& vulkan) {
			return Graphics::StagedBuffer(
				vulkan,
				vk::BufferUsageFlagBits::eUniformBuffer,
				UNIFORM_BUFFER_SIZE
			);
		}

		static vk::DescriptorSetLayout createDescriptorSetLayout(const Graphics::Vulkan& vulkan) {
			static const Utils::StaticId id;

			auto result = vulkan.createDescriptorSetLayout(id);

			if(!result) {
				//Create the bindings
				const std::array bindings = {
					vk::DescriptorSetLayoutBinding(	//UBO binding
						WINDOW_DESCRIPTOR_VIEWPORT,						//Binding
						vk::DescriptorType::eUniformBuffer,				//Type
						1,												//Count
						vk::ShaderStageFlagBits::eVertex,				//Shader stage
						nullptr											//Inmutable samplers
					), 
					vk::DescriptorSetLayoutBinding(	//UBO binding
						WINDOW_DESCRIPTOR_COLOR_TRANSFER,				//Binding
						vk::DescriptorType::eUniformBuffer,				//Type
						1,												//Count
						vk::ShaderStageFlagBits::eFragment,				//Shader stage
						nullptr											//Inmutable samplers
					), 
				};

				const vk::DescriptorSetLayoutCreateInfo createInfo(
					{},
					bindings.size(), bindings.data()
				);

				result = vulkan.createDescriptorSetLayout(id, createInfo);
			}

			return result;
		}

		static vk::UniqueDescriptorPool createDescriptorPool(const Graphics::Vulkan& vulkan){
			const std::array poolSizes = {
				vk::DescriptorPoolSize(
					vk::DescriptorType::eUniformBuffer,					//Descriptor type
					WINDOW_DESCRIPTOR_COUNT								//Descriptor count
				)
			};

			const vk::DescriptorPoolCreateInfo createInfo(
				{},														//Flags
				1,														//Descriptor set count
				poolSizes.size(), poolSizes.data()						//Pool sizes
			);

			return vulkan.createDescriptorPool(createInfo);
		}

		static vk::DescriptorSet createDescriptorSet(	const Graphics::Vulkan& vulkan,
														vk::DescriptorPool pool )
		{
			const std::array layouts {
				createDescriptorSetLayout(vulkan)
			};

			const vk::DescriptorSetAllocateInfo allocInfo(
				pool,													//Pool
				layouts.size(), layouts.data()							//Layouts
			);

			//Allocate it
			vk::DescriptorSet descriptorSet;
			static_assert(layouts.size() == 1);
			const auto result = vulkan.getDevice().allocateDescriptorSets(&allocInfo, &descriptorSet, vulkan.getDispatcher());

			if(result != vk::Result::eSuccess){
				throw Exception("Error allocating descriptor sets");
			}

			return descriptorSet;
		}

		static Graphics::Frame::Geometry createGeometry(std::byte* data,
														ScalingMode scalingMode,
														Math::Vec2f size )
		{
			return Graphics::Frame::Geometry(
				data, sizeof(Vertex), offsetof(Vertex, position), offsetof(Vertex, texCoord),
				scalingMode, size
			);
		}


		static vk::UniqueSwapchainKHR createSwapchain(	const Graphics::Vulkan& vulkan, 
														vk::SurfaceKHR surface, 
														vk::Extent2D& extent, 
														vk::Format format,
														vk::ColorSpaceKHR colorSpace,
														vk::SwapchainKHR old )
		{
			const auto& physicalDevice = vulkan.getPhysicalDevice();

			if(!physicalDevice.getSurfaceSupportKHR(0, surface, vulkan.getDispatcher())){
				throw Exception("Window surface not suppoted by the physical device");
			}

			const auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface, vulkan.getDispatcher());
			const auto imageCount = getImageCount(capabilities);
			extent = getExtent(capabilities, extent);

			const auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface, vulkan.getDispatcher());
			const auto surfaceFormat = getSurfaceFormat(surfaceFormats, vk::SurfaceFormatKHR(format, colorSpace));

			const auto queueFamilies = getQueueFamilies(vulkan);
			const auto sharingMode = (queueFamilies.size() > 1)
										? vk::SharingMode::eConcurrent
										: vk::SharingMode::eExclusive;
			
			const auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface, vulkan.getDispatcher());
			const auto presentMode = getPresentMode(presentModes);

			const vk::SwapchainCreateInfoKHR createInfo(
				{},													//Flags
				surface,											//Sufrace
				imageCount,											//Image count
				surfaceFormat.format,								//Image format
				surfaceFormat.colorSpace,							//Image color space
				extent,												//Image extent
				1,													//Image layer count
				vk::ImageUsageFlagBits::eColorAttachment,			//Image usage
				sharingMode,										//Sharing
				queueFamilies.size(), queueFamilies.data(),			//Used queue families
				capabilities.currentTransform,						//Transformations
				vk::CompositeAlphaFlagBitsKHR::eOpaque,				//Compositing
				presentMode,										//Present mode
				true,												//Clipping
				old													//Old swapchain
			);

			return vulkan.createSwapchain(createInfo);
		}

		static std::vector<vk::UniqueImageView> createImageViews(	const Graphics::Vulkan& vulkan,
																	vk::SwapchainKHR swapchain,
																	vk::Format format ) 
		{
			const auto images = vulkan.getDevice().getSwapchainImagesKHR(swapchain, vulkan.getDispatcher());
			std::vector<vk::UniqueImageView> result(images.size());

			for(size_t i = 0; i < images.size(); i++){
				const vk::ImageViewCreateInfo createInfo(
					{},												//Flags
					images[i],										//Image
					vk::ImageViewType::e2D,							//ImageView type
					format,											//Image format
					{},												//Swizzle
					vk::ImageSubresourceRange(						//Image subresources
						vk::ImageAspectFlagBits::eColor,				//Aspect mask
						0, 1, 0, 1										//Base mipmap level, mipmap levels, base array layer, layers
					)
				);

				result[i] =  vulkan.createImageView(createInfo);
			}

			return result;
		}

		static vk::UniqueRenderPass createRenderPass(	const Graphics::Vulkan& vulkan, 
														vk::Format format )
		{
			const std::array attachments = {
				vk::AttachmentDescription(
					{},												//Flags
					format,											//Attachemnt format
					vk::SampleCountFlagBits::e1,					//Sample count
					vk::AttachmentLoadOp::eClear,					//Color attachment load operation
					vk::AttachmentStoreOp::eStore,					//Color attachemnt store operation
					vk::AttachmentLoadOp::eDontCare,				//Stencil attachment load operation
					vk::AttachmentStoreOp::eDontCare,				//Stencil attachment store operation
					vk::ImageLayout::eUndefined,					//Initial layout
					vk::ImageLayout::ePresentSrcKHR					//Final layout
				)
			};

			const std::array attachmentReferences = {
				vk::AttachmentReference(
					0, 												//Attachments index
					vk::ImageLayout::eColorAttachmentOptimal 		//Attachemnt layout
				)
			};

			const std::array subpasses = {
				vk::SubpassDescription(
					{},												//Flags
					vk::PipelineBindPoint::eGraphics,				//Pipeline bind point
					0, nullptr,										//Input attachments
					attachmentReferences.size(), attachmentReferences.data(), //Color attachments
					nullptr,										//Resolve attachemnts
					nullptr,										//Depth / Stencil attachemnts
					0, nullptr										//Preserve attachments
				)
			};

			const std::array subpassDependencies = {
				vk::SubpassDependency(
					VK_SUBPASS_EXTERNAL,							//Source subpass
					0,												//Destination subpass
					vk::PipelineStageFlagBits::eColorAttachmentOutput,//Source stage
					vk::PipelineStageFlagBits::eColorAttachmentOutput,//Destination stage
					{},												//Source access mask
					vk::AccessFlagBits::eColorAttachmentRead | 		//Destintation access mask
						vk::AccessFlagBits::eColorAttachmentWrite
				)
			};

			const vk::RenderPassCreateInfo createInfo(
				{},													//Flags
				attachments.size(), attachments.data(),				//Attachemnts
				subpasses.size(), subpasses.data(),					//Subpasses
				subpassDependencies.size(), subpassDependencies.data()//Subpass dependencies
			);

			return vulkan.createRenderPass(createInfo);
		}

		static std::vector<vk::UniqueFramebuffer> createFramebuffers(	const Graphics::Vulkan& vulkan,
																		vk::RenderPass renderPass,
																		const std::vector<vk::UniqueImageView>& imageViews,
																		vk::Extent2D extent )
		{
			std::vector<vk::UniqueFramebuffer> result(imageViews.size());

			for(size_t i = 0; i < result.size(); i++){
				const std::array attachments = {
					*imageViews[i]
				};

				const vk::FramebufferCreateInfo createInfo(
					{},
					renderPass,
					attachments.size(),
					attachments.data(),
					extent.width, extent.height,
					1
				);

				result[i] = vulkan.createFramebuffer(createInfo);
			}

			return result;
		}

		static vk::PipelineLayout createPipelineLayout(	const Graphics::Vulkan& vulkan, 
														vk::Filter filter ) 
		{
			static std::array<Utils::StaticId, Graphics::Frame::FILTER_COUNT> ids;
			const auto layoutId = ids[static_cast<size_t>(filter)];

			auto result = vulkan.createPipelineLayout(layoutId);

			if(!result) {
				const std::array layouts = {
					createDescriptorSetLayout(vulkan),
					Graphics::Frame::getDescriptorSetLayout(vulkan, filter)
				};

				const vk::PipelineLayoutCreateInfo createInfo(
					{},													//Flags
					layouts.size(), layouts.data(),						//Descriptor set layouts
					0, nullptr											//Push constants
				);

				result = vulkan.createPipelineLayout(layoutId, createInfo);
			}

			return result;
		}

		static vk::UniquePipeline createPipeline(	const Graphics::Vulkan& vulkan,
													vk::RenderPass renderPass,
													vk::PipelineLayout layout,
													vk::Extent2D extent )
		{
			static //So that its ptr can be used as an indentifier
			#include <window_vert.h>
			const size_t vertId = reinterpret_cast<uintptr_t>(window_vert);
			static
			#include <window_frag.h>
			const size_t fragId = reinterpret_cast<uintptr_t>(window_frag);

			//Try to retrive modules from cache
			auto vertexShader = vulkan.createShaderModule(vertId);
			auto fragmentShader = vulkan.createShaderModule(fragId);

			if(!vertexShader || !fragmentShader) {
				//Modules aren't in cache. Create them
				vertexShader = vulkan.createShaderModule(vertId, window_vert);
				fragmentShader = vulkan.createShaderModule(fragId, window_frag);
			}

			constexpr auto SHADER_ENTRY_POINT = "main";
			const std::array shaderStages = {
				vk::PipelineShaderStageCreateInfo(		
					{},												//Flags
					vk::ShaderStageFlagBits::eVertex,				//Shader type
					vertexShader,									//Shader handle
					SHADER_ENTRY_POINT ),							//Shader entry point
				vk::PipelineShaderStageCreateInfo(		
					{},												//Flags
					vk::ShaderStageFlagBits::eFragment,				//Shader type
					fragmentShader,									//Shader handle
					SHADER_ENTRY_POINT ),							//Shader entry point
			};

			constexpr std::array vertexBindings = {
				vk::VertexInputBindingDescription(
					VERTEX_BUFFER_BINDING,
					sizeof(Vertex),
					vk::VertexInputRate::eVertex
				)
			};

			constexpr std::array vertexAttributes = {
				vk::VertexInputAttributeDescription(
					VERTEX_POSITION,
					VERTEX_BUFFER_BINDING,
					vk::Format::eR32G32Sfloat,
					offsetof(Vertex, position)
				),
				vk::VertexInputAttributeDescription(
					VERTEX_TEXCOORD,
					VERTEX_BUFFER_BINDING,
					vk::Format::eR32G32Sfloat,
					offsetof(Vertex, texCoord)
				)
			};

			const vk::PipelineVertexInputStateCreateInfo vertexInput(
				{},
				vertexBindings.size(), vertexBindings.data(),		//Vertex bindings
				vertexAttributes.size(), vertexAttributes.data()	//Vertex attributes
			);

			constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
				{},													//Flags
				vk::PrimitiveTopology::eTriangleStrip,				//Topology
				false												//Restart enable
			);

			const std::array viewports = {
				vk::Viewport(
					0.0f, 0.0f,										//Origin
					static_cast<float>(extent.width), 				//Width
					static_cast<float>(extent.height),				//Height
					0.0f, 1.0f										//min, max depth
				),
			};

			const std::array scissors = {
				vk::Rect2D(
					{ 0, 0 },										//Origin
					extent											//Size
				),
			};

			const vk::PipelineViewportStateCreateInfo viewport(
				{},													//Flags
				viewports.size(), viewports.data(),					//Viewports
				scissors.size(), scissors.data()					//Scissors
			);

			constexpr vk::PipelineRasterizationStateCreateInfo rasterizer(
				{},													//Flags
				false, 												//Depth clamp enabled
				false,												//Rasterizer discard enable
				vk::PolygonMode::eFill,								//Plygon mode
				vk:: CullModeFlagBits::eNone, 						//Cull faces
				vk::FrontFace::eClockwise,							//Front face direction
				false, 0.0f, 0.0f, 0.0f,							//Depth bias
				1.0f												//Line width
			);

			constexpr vk::PipelineMultisampleStateCreateInfo multisample(
				{},													//Flags
				vk::SampleCountFlagBits::e1,						//Sample count
				false, 1.0f,										//Sample shading enable, min sample shading
				nullptr,											//Sample mask
				false, false										//Alpha to coverage, alpha to 1 enable
			);

			constexpr vk::PipelineDepthStencilStateCreateInfo depthStencil(
				{},													//Flags
				false, false, 										//Depth test enable, write
				vk::CompareOp::eNever, 								//Depth compare op
				false,												//Depth bounds test
				false, 												//Stencil enabled
				{}, {},												//Stencil operation stete front, back
				0.0f, 0.0f											//min, max depth bounds
			);

			constexpr auto colorWriteMask = 
					vk::ColorComponentFlagBits::eR |				
					vk::ColorComponentFlagBits::eG |
					vk::ColorComponentFlagBits::eB |
					vk::ColorComponentFlagBits::eA ;

			constexpr std::array colorBlendAttachments = {
				vk::PipelineColorBlendAttachmentState(
					true,											//Enabled
					//Cf' = Ai*Ci + (1.0-Ai)*Cf; Typical color mixing equation
					vk::BlendFactor::eSrcAlpha,						//Source color weight
					vk::BlendFactor::eOneMinusSrcAlpha,				//Destination color weight
					vk::BlendOp::eAdd,								//Color operation
					//Af' = Ai + (1.0-Ai)*Af = Ai + Af - Ai*Af; So that Af' is always greater than Af and Ai
					//https://www.wolframalpha.com/input/?i=plot+%7C+x+%2B+%281-x%29+*+y+%7C+x+%3D+0+to+1+y+%3D+0+to+1
					vk::BlendFactor::eOne,							//Source alpha weight
					vk::BlendFactor::eOneMinusSrcAlpha,				//Destination alpha weight
					vk::BlendOp::eAdd,								//Alpha operation
					colorWriteMask									//Color write mask
				)
			};

			const vk::PipelineColorBlendStateCreateInfo colorBlend(
				{},													//Flags
				false,												//Enable logic operation
				vk::LogicOp::eCopy,									//Logic operation
				colorBlendAttachments.size(), colorBlendAttachments.data() //Blend attachments
			);

			constexpr vk::PipelineDynamicStateCreateInfo dynamicState(
				{},													//Flags
				0, nullptr											//Dynamis states
			);

			static const Utils::StaticId pipelineId;
			const vk::GraphicsPipelineCreateInfo createInfo(
				{},													//Flags
				shaderStages.size(), shaderStages.data(),			//Shader stages
				&vertexInput,										//Vertex input
				&inputAssembly,										//Vertex assembly
				nullptr,											//Tesselation
				&viewport,											//Viewports
				&rasterizer,										//Rasterizer
				&multisample,										//Multisampling
				&depthStencil,										//Depth / Stencil tests
				&colorBlend,										//Color blending
				&dynamicState,										//Dynamic states
				layout,												//Pipeline layout
				renderPass, 0,										//Renderpasses
				nullptr, static_cast<uint32_t>(pipelineId)			//Inherit
			);

			return vulkan.createGraphicsPipeline(createInfo);
		}



		static vk::Extent2D getExtent(	const vk::SurfaceCapabilitiesKHR& cap, 
										vk::Extent2D windowExtent )
		{
			constexpr auto INVALID_EXTENT = vk::Extent2D(
				std::numeric_limits<uint32_t>::max(), 
				std::numeric_limits<uint32_t>::max()
			);

			if(cap.currentExtent != INVALID_EXTENT){
				return cap.currentExtent;
			} else {
				return vk::Extent2D(
					Math::clamp(cap.minImageExtent.width, windowExtent.width, cap.maxImageExtent.width),
					Math::clamp(cap.minImageExtent.height, windowExtent.height, cap.maxImageExtent.height)
				);
			}
		}

		static vk::SurfaceFormatKHR getSurfaceFormat(	const std::vector<vk::SurfaceFormatKHR>& formats,
														vk::SurfaceFormatKHR desired )
		{
			if(std::find(formats.cbegin(), formats.cend(), desired) != formats.cend()){
				return desired;
			}

			throw Exception("Unsupported format!");
		}

		static uint32_t getImageCount(const vk::SurfaceCapabilitiesKHR& cap){
			const uint32_t desired = cap.minImageCount + 1;

			if(cap.maxImageCount){
				return std::min(desired, cap.maxImageCount);
			} else {
				return desired;
			}
		}

		static vk::PresentModeKHR getPresentMode(const std::vector<vk::PresentModeKHR>& presentModes){
			const std::array prefered = {
				vk::PresentModeKHR::eMailbox,
				vk::PresentModeKHR::eFifo //Required to be supported.
			};

			for(auto mode : prefered){
				if(std::find(presentModes.cbegin(), presentModes.cend(), mode) != presentModes.cend()){
					return mode; //Found a apropiate one
				}
			}

			throw Exception("No compatible presentation mode was found");
		}

		static std::vector<uint32_t> getQueueFamilies(const Graphics::Vulkan& vulkan){
			const std::set<uint32_t> families = {
				vulkan.getGraphicsQueueIndex(),
				vulkan.getPresentationQueueIndex()
			};

			return std::vector<uint32_t>(families.cbegin(), families.cend());
		}
	};

	Instance&									instance;

	std::string									windowName;
	Math::Vec2i 								size;
	Math::Vec2i 								position;
	State 										state;
	float										opacity;
	bool										resizeable;
	bool										decorated;

	Callbacks									callbacks;
	
	Signal::Input<Video>						videoIn;
	std::unique_ptr<Open>						opened;
	bool										hasChanged;

	static constexpr auto PRIORITY = Instance::OUTPUT_PRIORITY;
	static constexpr auto NO_POSTION = Math::Vec2i(std::numeric_limits<int32_t>::min());

	Impl(	Instance& instance,
			Math::Vec2i size,
			Callbacks callbacks)
		: instance(instance)
		, windowName(instance.getApplicationInfo().name)
		, size(size)
		, position(NO_POSTION)
		, state(State::NORMAL)
		, opacity(1.0f)
		, resizeable(true)
		, decorated(true)
		, callbacks(std::move(callbacks))
		, videoIn(std::string(Signal::makeInputName<Video>()))
	{
	}
	~Impl() = default;



	void open(ZuazoBase& base) 
	{
		assert(!opened);

		Window& win = static_cast<Window&>(base);
		const auto& videoMode = win.getVideoMode();

		const auto& vulkan = instance.getVulkan();
		auto [format, colorSpace, colorTransfer] = convertParameters(vulkan, videoMode);

		//Try to open it
		opened = std::make_unique<Impl::Open>(
			vulkan,
			size,
			windowName,
			createCallbacks(),
			format,
			colorSpace,
			std::move(colorTransfer),
			win.getScalingMode(),
			win.getScalingFilter()
		);
		
		//Set everything as desired
		//opened->window.setName(windowName); //Already set when constructing
		//opened->window.setSize(size); //Already set when constructing
		if(position != NO_POSTION) opened->window.setPosition(position);
		opened->window.setState(toGLFW(state));
		opened->window.setOpacity(opacity);
		opened->window.setResizeable(resizeable);
		opened->window.setDecorated(decorated);

		win.enablePeriodicUpdate(PRIORITY, getPeriod(videoMode.getFrameRateValue()));
		win.setVideoModeCompatibility(getVideoModeCompatibility());

		hasChanged = true;
	}

	void close(ZuazoBase& base) {
		assert(opened);

		Window& win = static_cast<Window&>(base);

		win.disablePeriodicUpdate();
		opened.reset();
		win.setVideoModeCompatibility(getVideoModeCompatibility());
	}


	void setVideoMode(VideoBase& base, const VideoMode& videoMode) {
		if(opened) {
			Window& win = static_cast<Window&>(base);

			auto [format, colorSpace, colorTransfer] = convertParameters(instance.getVulkan(), videoMode);
			opened->reconfigure(format, colorSpace, std::move(colorTransfer));

			win.disablePeriodicUpdate();
			win.enablePeriodicUpdate(PRIORITY, getPeriod(videoMode.getFrameRateValue()));
		}
	}

	void setScalingMode(VideoScalerBase&, ScalingMode mode) {
		if(opened) {
			opened->setScalingMode(mode);
		}
	}

	void setScalingFilter(VideoScalerBase&, ScalingFilter filter) {
		if(opened) {
			opened->setScalingFilter(filter);
		}
	}

	void update() {
		assert(opened);

		if(hasChanged || videoIn.hasChanged()) {
			//Input has changed, pull a frame from it
			const auto& frame = videoIn.pull();
			opened->draw(frame);
		}

		hasChanged = false;
	}

	std::vector<VideoMode> getVideoModeCompatibility() const {
		std::vector<VideoMode> result;

		if(opened) {
			const VideoMode baseCompatibility(
				Utils::Any<Rate>(),
				Utils::MustBe<Resolution>(opened->window.getResolution()),
				Utils::MustBe<AspectRatio>(AspectRatio(1, 1)),
				Utils::Any<ColorPrimaries>(),
				Utils::MustBe<ColorModel>(ColorModel::RGB),
				Utils::Any<ColorTransferFunction>(),
				Utils::MustBe<ColorSubsampling>(ColorSubsampling::RB_444),
				Utils::MustBe<ColorRange>(ColorRange::FULL),
				Utils::Any<ColorFormat>()
			);


			//Query for full compatibility
			const auto& vulkan = instance.getVulkan();
			const auto surfaceFormats = vulkan.getPhysicalDevice().getSurfaceFormatsKHR(*(opened->surface), vulkan.getDispatcher());

			for(const auto& surfaceFormat : surfaceFormats) {
				const auto [colorPrimary, colorTransferFunction] = Graphics::fromVulkan(surfaceFormat.colorSpace);
				const auto [format, colorTransferFunction2] = Graphics::fromVulkan(surfaceFormat.format);

				//Evaluate if it is a valid option
				if(	(colorPrimary != ColorPrimaries::NONE) &&
					(colorTransferFunction != ColorTransferFunction::NONE) &&
					(format != ColorFormat::NONE) )
				{
					//Copy the base compatibility in order to modify it
					VideoMode compatibility = baseCompatibility; 

					compatibility.setColorPrimaries(Utils::MustBe<ColorPrimaries>(colorPrimary));
					compatibility.setColorTransferFunction(Utils::MustBe<ColorTransferFunction>(colorTransferFunction));
					compatibility.setColorFormat(Utils::MustBe<ColorFormat>(format));

					result.emplace_back(std::move(compatibility));
				}
			}
		} else {
			/*
			 * This base compatibility has been checked here:
			 * http://www.vulkan.gpuinfo.org
			 */

			constexpr ColorFormat supportedFormat = 
				#ifdef __ANDROID__
					ColorFormat::R8G8B8A8;
				#else
					ColorFormat::B8G8R8A8;
				#endif	

			const VideoMode compatibility(
				Utils::Any<Rate>(),
				Utils::Any<Resolution>(),
				Utils::MustBe<AspectRatio>(AspectRatio(1, 1)),
				Utils::MustBe<ColorPrimaries>(ColorPrimaries::BT709),
				Utils::MustBe<ColorModel>(ColorModel::RGB),
				Utils::MustBe<ColorTransferFunction>(ColorTransferFunction::IEC61966_2_1),
				Utils::MustBe<ColorSubsampling>(ColorSubsampling::RB_444),
				Utils::MustBe<ColorRange>(ColorRange::FULL),
				Utils::MustBe<ColorFormat>(supportedFormat)
			);

			result.emplace_back(compatibility);
		}

		return result;
	}



	void setWindowName(std::string name) {
		if(windowName != name) {
			windowName = std::move(name);
			if(opened) opened->window.setName(windowName);
		}
	}

	const std::string& getWindowName() const {
		return windowName;
	}


	void setSize(Math::Vec2i s) {
		if(size != s) {
			size = s;
			if(opened) opened->window.setSize(size);
		}
	}

	Math::Vec2i getSize() const {
		return size;
	}

	void setSizeCallback(SizeCallback cbk) {
		callbacks.sizeCbk = std::move(cbk);
	}

	const SizeCallback& getSizeCallback() const {
		return callbacks.sizeCbk;
	}


	void setPosition(Math::Vec2i pos) {
		if(position != pos) {
			position = pos;
			if(opened) opened->window.setPosition(position);
		}
	}

	Math::Vec2i getPosition() const {
		return position;
	}

	void setPositionCallback(PositionCallback cbk) {
		callbacks.positionCbk = std::move(cbk);
	}

	const PositionCallback& getPositionCallback() const {
		return callbacks.positionCbk;
	}


	void setState(State st) {
		if(state != st) {
			state = st;
			if(opened) opened->window.setState(toGLFW(state));
		}
	}

	State getState() const {
		return state;
	}

	void setStateCallback(StateCallback cbk) {
		callbacks.stateCbk = std::move(cbk);
	}

	const StateCallback& getStateCallback() const {
		return callbacks.stateCbk;
	}


	Math::Vec2f getScale() const {
		return opened ? opened->window.getScale() : Math::Vec2f(0.0f);
	}

	void setScaleCallback(ScaleCallback cbk) {
		callbacks.scaleCbk = std::move(cbk);
	}

	const ScaleCallback& getScaleCallback() const {
		return callbacks.scaleCbk;
	}


	void focus() {
		if(opened) opened->window.focus();
	}

	void setFocusCallback(FocusCallback cbk) {
		callbacks.focusCbk = std::move(cbk);
	}

	const FocusCallback& getFocusCallback() const {
		return callbacks.focusCbk;
	}


	bool shouldClose() const {
		return opened ? opened->window.shouldClose() : false;
	}

	void setShouldCloseCallback(ShouldCloseCallback cbk) {
		callbacks.shouldCloseCbk = std::move(cbk);
	}

	const ShouldCloseCallback& getShouldCloseCallback() const {
		return callbacks.shouldCloseCbk;
	}


	void setOpacity(float opa) {
		opacity = opa;
		if(opened) opened->window.setOpacity(opacity);
	}

	float getOpacity() const {
		return opacity;
	}


	void setResizeable(bool res) {
		resizeable = res;
		if(opened) opened->window.setResizeable(resizeable);
	}

	bool getResizeable() const {
		return resizeable;
	}


	void setDecorated(bool deco) {
		decorated = deco;
		if(opened) opened->window.setDecorated(decorated);
	}

	bool getDecorated() const {
		return decorated;
	}



private:
	GLFW::Window::Callbacks createCallbacks() {
		return {
			std::bind(&Impl::stateCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::positionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::sizeCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::resolutionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::scaleCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::shouldCloseCallback, std::ref(*this)),
			GLFW::Window::RefreshCallback(),
			std::bind(&Impl::focusCallback, std::ref(*this), std::placeholders::_1)
		};
	}

	void resolutionCallback(Resolution res) {
		assert(opened);
		std::lock_guard<Instance> lock(instance);
		opened->resizeFramebuffer(res);
		//TODO update compatibility and remove the above line

		hasChanged = true;
	}

	void sizeCallback(Math::Vec2i s) {
		std::lock_guard<Instance> lock(instance);

		size = s;
		if(callbacks.sizeCbk) {
			callbacks.sizeCbk(size);
		}
	}

	void positionCallback(Math::Vec2i pos) {
		std::lock_guard<Instance> lock(instance);

		position = pos;
		if(callbacks.positionCbk) {
			callbacks.positionCbk(position);
		}
	}

	void stateCallback(GLFW::Window::State st) {
		std::lock_guard<Instance> lock(instance);

		state = fromGLFW(st);
		if(callbacks.stateCbk) {
			callbacks.stateCbk(state);
		}
	}

	void scaleCallback(Math::Vec2f sc) {
		std::lock_guard<Instance> lock(instance);

		if(callbacks.scaleCbk) {
			callbacks.scaleCbk(sc);
		}
	}

	void focusCallback(bool foc) {
		std::lock_guard<Instance> lock(instance);

		if(callbacks.focusCbk) {
			callbacks.focusCbk(foc);
		}
	}

	void shouldCloseCallback() {
		std::lock_guard<Instance> lock(instance);

		if(callbacks.shouldCloseCbk) {
			callbacks.shouldCloseCbk();
		}
	}



	static std::tuple<vk::Format, vk::ColorSpaceKHR, Graphics::ColorTransfer>
	convertParameters(	const Graphics::Vulkan& vulkan,
						const VideoMode& videoMode )
	{
		const auto frameDescriptor = videoMode.getFrameDescriptor();

		//Obatin the pixel format
		auto formats = Graphics::Frame::getPlaneDescriptors(frameDescriptor);
		assert(formats.size() == 1);

		auto& f = formats[0];
		std::tie(f.format, f.swizzle) = Graphics::optimizeFormat(std::make_tuple(f.format, f.swizzle));
		assert(formats[0].swizzle == vk::ComponentMapping());

		//Obtain the color space
		const auto colorSpace = Graphics::toVulkan(
			videoMode.getColorPrimariesValue(), 
			videoMode.getColorTransferFunctionValue()
		);

		//Create the color transfer characteristics
		Graphics::ColorTransfer colorTransfer(frameDescriptor);

		const auto& supportedFormats = vulkan.getFormatSupport().framebuffer;
		colorTransfer.optimize(formats, supportedFormats);

		return std::make_tuple(f.format, colorSpace, std::move(colorTransfer));
	}

	static constexpr GLFW::Window::State toGLFW(Window::State state) {
		switch(state) {
		case Window::State::NORMAL:		return GLFW::Window::State::NORMAL;
		case Window::State::HIDDEN:		return GLFW::Window::State::HIDDEN;
		case Window::State::FULLSCREEN:	return GLFW::Window::State::FULLSCREEN;
		case Window::State::ICONIFIED:	return GLFW::Window::State::ICONIFIED;
		case Window::State::MAXIMIZED:	return GLFW::Window::State::MAXIMIZED;
		default: return static_cast<GLFW::Window::State>(state);
		}
	}

	static constexpr Window::State fromGLFW(GLFW::Window::State state) {
		switch(state) {
		case GLFW::Window::State::NORMAL:		return Window::State::NORMAL;
		case GLFW::Window::State::HIDDEN:		return Window::State::HIDDEN;
		case GLFW::Window::State::FULLSCREEN:	return Window::State::FULLSCREEN;
		case GLFW::Window::State::ICONIFIED:	return Window::State::ICONIFIED;
		case GLFW::Window::State::MAXIMIZED:	return Window::State::MAXIMIZED;
		default: return static_cast<Window::State>(state);
		}
	}
};


/*
 * Window
 */

Window::Window(	Instance& instance, 
				std::string name, 
				Math::Vec2i size,
				VideoMode videoMode,
				Callbacks cbks )
	: ZuazoBase(instance, std::move(name))
	, VideoBase(std::move(videoMode))
	, VideoScalerBase()
	, m_impl({}, instance, size, std::move(cbks))
{
	Layout::registerPad(m_impl->videoIn);
	setOpenCallback(std::bind(&Impl::open, std::ref(*m_impl), std::placeholders::_1));
	setCloseCallback(std::bind(&Impl::close, std::ref(*m_impl), std::placeholders::_1));
	setUpdateCallback(std::bind(&Impl::update, std::ref(*m_impl)));
	setVideoModeCompatibility(m_impl->getVideoModeCompatibility());
	setVideoModeCallback(std::bind(&Impl::setVideoMode, std::ref(*m_impl), std::placeholders::_1, std::placeholders::_2));
	setScalingModeCallback(std::bind(&Impl::setScalingMode, std::ref(*m_impl), std::placeholders::_1, std::placeholders::_2));
	setScalingFilterCallback(std::bind(&Impl::setScalingFilter, std::ref(*m_impl), std::placeholders::_1, std::placeholders::_2));
}

Window::Window(Window&& other) = default;

Window::~Window() = default;

Window& Window::operator=(Window&& other) = default;


void Window::setWindowName(std::string name) {
	m_impl->setWindowName(std::move(name));
}
const std::string& Window::getWindowName() const {
	return m_impl->getWindowName();
}


void Window::setSize(Math::Vec2i size) {
	m_impl->setSize(size);
}

Math::Vec2i Window::getSize() const {
	return m_impl->getSize();
}

void Window::setSizeCallback(SizeCallback cbk) {
	m_impl->setSizeCallback(std::move(cbk));
}

const Window::SizeCallback& Window::getSizeCallback() const {
	return m_impl->getSizeCallback();
}


void Window::setPosition(Math::Vec2i pos) {
	m_impl->setPosition(pos);
}

Math::Vec2i Window::getPosition() const {
	return m_impl->getPosition();
}

void Window::setPositionCallback(PositionCallback cbk) {
	m_impl->setPositionCallback(std::move(cbk));
}

const Window::PositionCallback&	Window::getPositionCallback() const {
	return m_impl->getPositionCallback();
}


void Window::setState(State state) {
	m_impl->setState(state);
}

Window::State Window::getState() const {
	return m_impl->getState();
}

void Window::setStateCallback(StateCallback cbk) {
	m_impl->setStateCallback(std::move(cbk));
}

const Window::StateCallback& Window::getStateCallback() const {
	return m_impl->getStateCallback();
}


Math::Vec2f Window::getScale() const {
	return m_impl->getScale();
}

void Window::setScaleCallback(ScaleCallback cbk) {
	m_impl->setScaleCallback(std::move(cbk));
}

const Window::ScaleCallback& Window::getScaleCallback() const {
	return m_impl->getScaleCallback();
}


void Window::focus() {
	m_impl->focus();
}

void Window::setFocusCallback(FocusCallback cbk) {
	m_impl->setFocusCallback(std::move(cbk));
}

const Window::FocusCallback& Window::getFocusCallback() const {
	return m_impl->getFocusCallback();
}


bool Window::shouldClose() const {
	return m_impl->shouldClose();
}

void Window::setShouldCloseCallback(ShouldCloseCallback cbk) {
	m_impl->setShouldCloseCallback(std::move(cbk));
}

const Window::ShouldCloseCallback& Window::getShouldCloseCallback() const {
	return m_impl->getShouldCloseCallback();
}


void Window::setOpacity(float opa) {
	m_impl->setOpacity(opa);
}

float Window::getOpacity() const {
	return m_impl->getOpacity();
}


void Window::setResizeable(bool resizeable) {
	m_impl->setResizeable(resizeable);
}

bool Window::getResizeable() const {
	return m_impl->getResizeable();
}


void Window::setDecorated(bool deco) {
	m_impl->setDecorated(deco);
}

bool Window::getDecorated() const {
	return m_impl->getDecorated();
}



void Window::init() {
	GLFW::init();
}

//std::vector<Window::Monitor> Window::getMonitors();

void Window::pollEvents(std::unique_lock<Instance>& lock) {
	assert(lock.owns_lock());
	lock.unlock();

	GLFW::getGLFW().pollEvents();

	lock.lock();
}

void Window::waitEvents(std::unique_lock<Instance>& lock) {
	assert(lock.owns_lock());
	lock.unlock();

	GLFW::getGLFW().waitEvents();
	
	lock.lock();
}

void Window::waitEvents(std::unique_lock<Instance>& lock, Duration timeout) {
	assert(lock.owns_lock());
	lock.unlock();

	GLFW::getGLFW().waitEvents(timeout);
	
	lock.lock();
}

std::shared_ptr<Instance::ScheduledCallback> Window::enableRegularEventPolling(Instance& instance, Instance::Priority prior) {
	auto callback = std::make_shared<Instance::ScheduledCallback>(
		[&instance] {
			//As it is being called from the loop, instance should be locked by this thread
			std::unique_lock<Instance> lock(instance, std::adopt_lock);
			Window::pollEvents(lock);
			lock.release(); //Leave it locked
		}
	);

	instance.addRegularCallback(callback, prior);
	return callback;
}

}