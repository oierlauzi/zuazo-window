#include <zuazo/Consumers/Window.h>

#include "../GLFW.h"

#include <zuazo/Graphics/Vulkan.h>
#include <zuazo/Graphics/VulkanConversions.h>
#include <zuazo/Graphics/ColorTransfer.h>
#include <zuazo/Graphics/StagedBuffer.h>
#include <zuazo/Utils/Area.h>
#include <zuazo/Utils/CPU.h>
#include <zuazo/Utils/StaticId.h>
#include <zuazo/Utils/Functions.h>

#include <cmath>
#include <algorithm>
#include <limits>
#include <set>
#include <bitset>
#include <mutex>
#include <sstream>
#include <unordered_map>


namespace Zuazo::Consumers {

/*
 * Window::Monitor::Impl
 */

struct Window::Monitor::Impl {
	GLFW::Monitor monitor;

	Impl()
		: monitor(nullptr)
	{
	}

	Impl(GLFW::Monitor mon)
		: monitor(std::move(mon))
	{
	}

	~Impl() = default;

	std::string_view getName() const {
		return monitor.getName();
	}

	Math::Vec2d getPhysicalSize() const {
		return monitor.getPhysicalSize();
	}

	Math::Vec2i getSize() const {
		return monitor.getSize();
	}

	Math::Vec2i getPosition() const {
		return monitor.getPosition();
	}

	Rate getFrameRate() const {
		return Rate(monitor.getFrameRate(), 1);
	}

};

/*
 * Window::Monitor
 */

Window::Monitor::Monitor()
	: m_impl({})
{
}

Window::Monitor::Monitor(Utils::Pimpl<Impl> pimpl)
	: m_impl(std::move(pimpl))
{
}

Window::Monitor::Monitor(Monitor&& other) = default;

Window::Monitor::~Monitor() = default;

Window::Monitor& Window::Monitor::operator=(Monitor&& other) = default;



std::string_view Window::Monitor::getName() const {
	return m_impl->getName();
}

Math::Vec2d Window::Monitor::getPhysicalSize() const {
	return m_impl->getPhysicalSize();
}

Math::Vec2i Window::Monitor::getSize() const {
	return m_impl->getSize();
}

Math::Vec2i Window::Monitor::getPosition() const {
	return m_impl->getPosition();
}

Rate Window::Monitor::getFrameRate() const {
	return m_impl->getFrameRate();
}


/*
 * WindowImpl
 */
struct WindowImpl {
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

		using UniformBufferLayout = std::array<Utils::Area, WINDOW_DESCRIPTOR_COUNT>;


		static constexpr uint32_t VERTEX_BUFFER_BINDING = 0;
		static constexpr uint32_t VERTEX_POSITION = 0;
		static constexpr uint32_t VERTEX_TEXCOORD = 1;


		const Graphics::Vulkan&						vulkan;

		GLFW::Window								window;
		vk::UniqueSurfaceKHR						surface;
		vk::UniqueCommandPool						commandPool;
		vk::CommandBuffer							commandBuffer;
		Graphics::StagedBuffer						vertexBuffer;
		UniformBufferLayout							uniformBufferLayout;
		Graphics::StagedBuffer						uniformBuffer;
		vk::UniqueDescriptorPool					descriptorPool;
		vk::DescriptorSet							descriptorSet;
		vk::UniqueSemaphore 						imageAvailableSemaphore;
		vk::UniqueSemaphore							renderFinishedSemaphore;
		vk::UniqueFence								renderFinishedFence;

		vk::Extent2D								extent;
		vk::Format 									format;
		vk::ColorSpaceKHR 							colorSpace;
		Graphics::OutputColorTransfer				colorTransfer;
		vk::Filter									filter;
		Graphics::Frame::Geometry					geometry;

		vk::UniqueSwapchainKHR						swapchain;
		std::vector<vk::UniqueImageView>			swapchainImageViews;
		vk::RenderPass								renderPass;
		std::vector<vk::UniqueFramebuffer>			framebuffers;
		vk::PipelineLayout							pipelineLayout;
		vk::UniquePipeline							pipeline;


		Open(	const Graphics::Vulkan& vulkan,
				Math::Vec2i size,
				const char* name,
				GLFW::Monitor mon,
				GLFW::Window::Callbacks cbks,
				ScalingMode scalingMode,
				ScalingFilter scalingFilter ) 
			: vulkan(vulkan)
			, window(size, name, mon, std::move(cbks))
			, surface(window.getSurface(vulkan))
			, commandPool(createCommandPool(vulkan))
			, commandBuffer(createCommandBuffer(vulkan, *commandPool))
			, vertexBuffer(createVertexBuffer(vulkan))
			, uniformBufferLayout(createUniformBufferLayout(vulkan))
			, uniformBuffer(createUniformBuffer(vulkan, uniformBufferLayout))
			, descriptorPool(createDescriptorPool(vulkan))
			, descriptorSet(createDescriptorSet(vulkan, *descriptorPool))
			, imageAvailableSemaphore(vulkan.createSemaphore())
			, renderFinishedSemaphore(vulkan.createSemaphore())
			, renderFinishedFence(vulkan.createFence(true))

			, extent(Graphics::toVulkan(window.getResolution()))
			, format(vk::Format::eUndefined)
			, colorSpace(static_cast<vk::ColorSpaceKHR>(-1))
			, colorTransfer()
			, filter(Graphics::toVulkan(scalingFilter))
			, geometry(createGeometry(vertexBuffer.data(), scalingMode, Math::Vec2f(extent.width, extent.height)))

			, swapchain()
			, swapchainImageViews()
			, renderPass()
			, framebuffers()
			, pipelineLayout(createPipelineLayout(vulkan, filter))
			, pipeline()

		{
			writeDescriptorSets();
			updateUniforms();
		}

		~Open() {
			vertexBuffer.waitCompletion(vulkan);
			uniformBuffer.waitCompletion(vulkan);
			waitCompletion();
		}

		void reconfigure(vk::Extent2D ext, vk::Format fmt, vk::ColorSpaceKHR cs, Graphics::OutputColorTransfer ct) {
			enum {
				RECREATE_SWAPCHAIN,
				RECREATE_RENDERPASS,
				RECREATE_FRAMEBUFFERS,
				RECREATE_PIPELINE,
				UPDATE_GEOMETRY,
				UPDATE_VIEWPORT,
				UPDATE_COLOR_TRANSFER,

				MODIFICATION_COUNT
			};

			std::bitset<MODIFICATION_COUNT> modifications;

			if(extent != ext) {
				//Resolution has changed
				extent = ext;

				modifications.set(RECREATE_SWAPCHAIN);
				modifications.set(RECREATE_PIPELINE);
				modifications.set(UPDATE_GEOMETRY);
			}

			if(format != fmt) {
				//Format has changed
				format = fmt;

				modifications.set(RECREATE_SWAPCHAIN);
				modifications.set(RECREATE_RENDERPASS);
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

				if(modifications.test(RECREATE_SWAPCHAIN)) {
					const auto oldExtent = extent;
					swapchain = createSwapchain(vulkan, *surface, extent, format, colorSpace, *swapchain);
					swapchainImageViews = createImageViews(vulkan, *swapchain, format);
					
					modifications.set(RECREATE_FRAMEBUFFERS);

					//Extent might have changed
					if(oldExtent != extent) {
						modifications.set(RECREATE_PIPELINE);
						modifications.set(UPDATE_GEOMETRY);
					}
				} 

				if(modifications.test(RECREATE_RENDERPASS)) {
					renderPass = createRenderPass(vulkan, format);
					
					modifications.set(RECREATE_FRAMEBUFFERS);
					modifications.set(RECREATE_PIPELINE);
				} 

				if(modifications.test(RECREATE_FRAMEBUFFERS)) {
					framebuffers = createFramebuffers(vulkan, renderPass, swapchainImageViews, extent);
				} 

				if(modifications.test(RECREATE_PIPELINE)) {
					pipeline = createPipeline(vulkan, renderPass, pipelineLayout, extent);
				} 

				if(modifications.test(UPDATE_GEOMETRY)) {
					geometry.setTargetSize(Math::Vec2f(extent.width, extent.height));
					modifications.set(UPDATE_VIEWPORT);
				}

				//Evaluate which uniforms need to be updated
				if(modifications.test(UPDATE_VIEWPORT) && modifications.test(UPDATE_COLOR_TRANSFER)) updateUniforms();
				else if(modifications.test(UPDATE_VIEWPORT)) updateViewportUniform();
				else if(modifications.test(UPDATE_COLOR_TRANSFER)) updateColorTransferUniform();
			}
		}

		void setScalingMode(ScalingMode mode) {
			waitCompletion();
			geometry.setScalingMode(mode);
		}

		void setScalingFilter(ScalingFilter filt) {
			waitCompletion();
			filter = Graphics::toVulkan(filt);

			//Recreate the pipeline layout
			pipelineLayout = createPipelineLayout(vulkan, filter);
			pipeline = createPipeline(vulkan, renderPass, pipelineLayout, extent);
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
				vk::ClearValue(vk::ClearColorValue(std::array{ 0.0f, 0.0f, 0.0f, 0.0f }))
			};
			const vk::RenderPassBeginInfo rendBegin(
				renderPass,															//Renderpass
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

	private:
		void recreateSwapchain() {
			const auto oldExtent = extent;
			swapchain = createSwapchain(vulkan, *surface, extent, format, colorSpace, *swapchain);
			swapchainImageViews = createImageViews(vulkan, *swapchain, format);
			framebuffers = createFramebuffers(vulkan, renderPass, swapchainImageViews, extent);

			if(extent != oldExtent) {
				pipeline = createPipeline(vulkan, renderPass, pipelineLayout, extent);
				geometry.setTargetSize(Math::Vec2f(extent.width, extent.height));
				updateViewportUniform();
			}
		}

		void updateViewportUniform() {
			uniformBuffer.waitCompletion(vulkan);

			auto& size = *(reinterpret_cast<Math::Vec2f*>(uniformBufferLayout[WINDOW_DESCRIPTOR_VIEWPORT].begin(uniformBuffer.data())));
			size = geometry.getTargetSize();

			uniformBuffer.flushData(
				vulkan,
				uniformBufferLayout[WINDOW_DESCRIPTOR_VIEWPORT].offset(),
				uniformBufferLayout[WINDOW_DESCRIPTOR_VIEWPORT].size(),
				vulkan.getGraphicsQueueIndex(),
				vk::AccessFlagBits::eUniformRead,
				vk::PipelineStageFlagBits::eVertexShader
			);
		}

		void updateColorTransferUniform() {
			uniformBuffer.waitCompletion(vulkan);
			
			std::memcpy(
				uniformBufferLayout[WINDOW_DESCRIPTOR_COLOR_TRANSFER].begin(uniformBuffer.data()),
				colorTransfer.data(),
				uniformBufferLayout[WINDOW_DESCRIPTOR_COLOR_TRANSFER].size()
			);

			uniformBuffer.flushData(
				vulkan,
				uniformBufferLayout[WINDOW_DESCRIPTOR_COLOR_TRANSFER].offset(),
				uniformBufferLayout[WINDOW_DESCRIPTOR_COLOR_TRANSFER].size(),
				vulkan.getGraphicsQueueIndex(),
				vk::AccessFlagBits::eUniformRead,
				vk::PipelineStageFlagBits::eFragmentShader
			);
		}

		void updateUniforms() {
			uniformBuffer.waitCompletion(vulkan);		
			
			auto& size = *(reinterpret_cast<Math::Vec2f*>(uniformBufferLayout[WINDOW_DESCRIPTOR_VIEWPORT].begin(uniformBuffer.data())));
			size = geometry.getTargetSize();

			std::memcpy(
				uniformBufferLayout[WINDOW_DESCRIPTOR_COLOR_TRANSFER].begin(uniformBuffer.data()),
				colorTransfer.data(),
				uniformBufferLayout[WINDOW_DESCRIPTOR_COLOR_TRANSFER].size()
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
					uniformBuffer.getBuffer(),										//Buffer
					uniformBufferLayout[WINDOW_DESCRIPTOR_VIEWPORT].offset(),		//Offset
					uniformBufferLayout[WINDOW_DESCRIPTOR_VIEWPORT].size()			//Size
				)
			};
			const std::array colorTransferBuffers = {
				vk::DescriptorBufferInfo(
					uniformBuffer.getBuffer(),										//Buffer
					uniformBufferLayout[WINDOW_DESCRIPTOR_COLOR_TRANSFER].offset(),	//Offset
					uniformBufferLayout[WINDOW_DESCRIPTOR_COLOR_TRANSFER].size()	//Size
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
			if(vulkan.getDevice().allocateCommandBuffers(&allocInfo, &result, vulkan.getDispatcher()) != vk::Result::eSuccess) {
				throw Exception("Error allocating commnand buffers");
			}

			return result;
		}


		static Graphics::StagedBuffer createVertexBuffer(const Graphics::Vulkan& vulkan) {
			return Graphics::StagedBuffer(
				vulkan,
				vk::BufferUsageFlagBits::eVertexBuffer,
				sizeof(Vertex) * Graphics::Frame::Geometry::VERTEX_COUNT
			);
		}

		static UniformBufferLayout createUniformBufferLayout(const Graphics::Vulkan& vulkan) {
			const auto& limits = vulkan.getPhysicalDeviceProperties().limits;

			const auto viewportOff = 0;
			const auto colorTansferOff = Utils::align(viewportOff + sizeof(glm::vec2), limits.minUniformBufferOffsetAlignment);

			return UniformBufferLayout {
				Utils::Area(viewportOff, 		sizeof(glm::vec2)),		//Viewport
				Utils::Area(colorTansferOff,	Graphics::OutputColorTransfer::size() ) //Color Transfer
			};
		}


		static Graphics::StagedBuffer createUniformBuffer(const Graphics::Vulkan& vulkan, const UniformBufferLayout& layout) {
			return Graphics::StagedBuffer(
				vulkan,
				vk::BufferUsageFlagBits::eUniformBuffer,
				layout.back().end()
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
						nullptr											//Immutable samplers
					), 
					vk::DescriptorSetLayoutBinding(	//UBO binding
						WINDOW_DESCRIPTOR_COLOR_TRANSFER,				//Binding
						vk::DescriptorType::eUniformBuffer,				//Type
						1,												//Count
						vk::ShaderStageFlagBits::eFragment,				//Shader stage
						nullptr											//Immutable samplers
					), 
				};

				const vk::DescriptorSetLayoutCreateInfo createInfo(
					{},
					bindings.size(), bindings.data()
				);

				result = vulkan.createDescriptorSetLayout(id, createInfo);
			}

			assert(result);
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

		static vk::RenderPass createRenderPass(	const Graphics::Vulkan& vulkan, 
												vk::Format format )
		{
			static std::unordered_map<vk::Format, Utils::StaticId> ids;
			const auto id = ids[format].get();

			auto result = vulkan.createRenderPass(id);
			if(!result) {
				//Render pass was not created
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

				constexpr std::array attachmentReferences = {
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
						nullptr,										//Resolve attachments
						nullptr,										//Depth / Stencil attachments
						0, nullptr										//Preserve attachments
					)
				};

				constexpr std::array subpassDependencies = {
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
					attachments.size(), attachments.data(),				//Attachments
					subpasses.size(), subpasses.data(),					//Subpasses
					subpassDependencies.size(), subpassDependencies.data()//Subpass dependencies
				);

				result = vulkan.createRenderPass(id, createInfo);
			}

			assert(result);
			return result;
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
			static //So that its ptr can be used as an identifier
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
				vk::PolygonMode::eFill,								//Polygon mode
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
				{}, {},												//Stencil operation state front, back
				0.0f, 0.0f											//min, max depth bounds
			);

			constexpr std::array colorBlendAttachments = {
				Graphics::toVulkan(BlendingMode::WRITE) 			//Dont blend, just write to the framebuffer
			};

			const vk::PipelineColorBlendStateCreateInfo colorBlend(
				{},													//Flags
				false,												//Enable logic operation
				vk::LogicOp::eCopy,									//Logic operation
				colorBlendAttachments.size(), colorBlendAttachments.data() //Blend attachments
			);

			constexpr vk::PipelineDynamicStateCreateInfo dynamicState(
				{},													//Flags
				0, nullptr											//Dynamic states
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
			const std::array preferred = {
				vk::PresentModeKHR::eMailbox,
				vk::PresentModeKHR::eFifo //Required to be supported.
			};

			for(auto mode : preferred){
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

	Signal::Input<Video>						videoIn;

	std::reference_wrapper<Window>				owner;

	std::string									windowName;
	Math::Vec2i 								size;
	Math::Vec2i 								position;
	Window::State 								state;
	float										opacity;
	bool										resizeable;
	bool										decorated;
	GLFW::Monitor								monitor;

	Window::Callbacks							callbacks;
	
	std::unique_ptr<Open>						opened;
	bool										hasChanged;


	static constexpr auto PRIORITY = Instance::OUTPUT_PRIORITY;
	static constexpr auto NO_POSTION = Math::Vec2i(std::numeric_limits<int32_t>::min());

	WindowImpl(	Window& owner,
				Instance& instance,
				Math::Vec2i size,
				const Window::Monitor& mon,
				Window::Callbacks callbacks)
		: owner(owner)
		, windowName(instance.getApplicationInfo().getName())
		, size(size)
		, position(NO_POSTION)
		, state(Window::State::NORMAL)
		, opacity(1.0f)
		, resizeable(true)
		, decorated(true)
		, monitor(getGLFWMonitor(mon))
		, callbacks(std::move(callbacks))
	{
	}
	~WindowImpl() = default;


	void moved(ZuazoBase& base) {
		owner = static_cast<Window&>(base);
	}

	void open(ZuazoBase& base) {
		assert(!opened);
		assert(&owner.get() == &static_cast<Window&>(base));
		ZUAZO_IGNORE_PARAM(base);

		Window& win = owner.get();
		const auto& vulkan = win.getInstance().getVulkan();

		//Try to open it
		opened = std::make_unique<Open>(
			vulkan,
			size,
			windowName.c_str(),
			monitor,
			createCallbacks(),
			win.getScalingMode(),
			win.getScalingFilter()
		);
		
		//Set everything as desired
		//opened->window.setName(windowName); //Already set when constructing
		//opened->window.setSize(size); //Already set when constructing
		if(position != NO_POSTION) opened->window.setPosition(position);
		opened->window.setState(static_cast<GLFW::Window::State>(state));
		opened->window.setOpacity(opacity);
		opened->window.setResizeable(resizeable);
		opened->window.setDecorated(decorated);

		win.setVideoModeCompatibility(getVideoModeCompatibility());

		//Timing will be enabled on the setVideoMode() callback
		//win.enablePeriodicUpdate(PRIORITY, getPeriod(videoMode.getFrameRateValue())); 

		hasChanged = true;
	}

	void close(ZuazoBase& base) {
		assert(opened);
		assert(&owner.get() == &static_cast<Window&>(base));
		ZUAZO_IGNORE_PARAM(base);

		Window& win = owner.get();

		win.disablePeriodicUpdate();
		opened.reset();
		win.setVideoModeCompatibility(getVideoModeCompatibility());
		videoIn.reset();
	}


	void setVideoMode(VideoBase& base, const VideoMode& videoMode) {
		if(opened) {
			Window& win = static_cast<Window&>(base);

			auto [extent, format, colorSpace, colorTransfer] = convertParameters(win.getInstance().getVulkan(), videoMode);
			opened->reconfigure(extent, format, colorSpace, std::move(colorTransfer));

			win.disablePeriodicUpdate();
			win.enablePeriodicUpdate(PRIORITY, getPeriod(videoMode.getFrameRateValue()));

			hasChanged = true;
		}
	}

	void setScalingMode(VideoScalerBase&, ScalingMode mode) {
		if(opened) {
			opened->setScalingMode(mode);
			hasChanged = true;
		}
	}

	void setScalingFilter(VideoScalerBase&, ScalingFilter filter) {
		if(opened) {
			opened->setScalingFilter(filter);
			hasChanged = true;
		}
	}

	void update() {
		assert(opened);

		if(hasChanged || videoIn.hasChanged()) {
			//Input has changed, pull a frame from it
			const auto& frame = videoIn.pull();
			opened->draw(frame);

			hasChanged = false;
		}
	}

	std::vector<VideoMode> getVideoModeCompatibility() const {
		std::vector<VideoMode> result;

		if(opened) {
			//Select a monitor to depend on
			const auto& mon = monitor ? monitor : GLFW::getGLFW().getPrimaryMonitor();

			//Construct a base capability struct which will be common to all compatibilities
			const VideoMode baseCompatibility(
				Utils::Range<Rate>(Rate(0, 1), Rate(mon.getMode().frameRate, 1)),
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
			Window& win = owner.get();
			const auto& vulkan = win.getInstance().getVulkan();
			const auto surfaceFormats = vulkan.getPhysicalDevice().getSurfaceFormatsKHR(*(opened->surface), vulkan.getDispatcher());

			for(const auto& surfaceFormat : surfaceFormats) {
				const auto [colorPrimary, colorTransferFunction] = Graphics::fromVulkan(surfaceFormat.colorSpace);
				const auto [format, colorTransferFunction2] = Graphics::fromVulkan(surfaceFormat.format);

				//Evaluate if it is a valid option
				if(	(colorPrimary != ColorPrimaries::NONE) &&
					(colorTransferFunction != ColorTransferFunction::NONE) &&
					(format != ColorFormat::NONE) &&
					(colorTransferFunction2 == ColorTransferFunction::LINEAR) ) //To avoid duplicates
				{
					//Copy the base compatibility in order to modify it
					VideoMode compatibility = baseCompatibility; 

					compatibility.setColorPrimaries(Utils::MustBe<ColorPrimaries>(colorPrimary));
					compatibility.setColorTransferFunction(Utils::MustBe<ColorTransferFunction>(colorTransferFunction));
					compatibility.setColorFormat(Utils::MustBe<ColorFormat>(format));

					result.emplace_back(std::move(compatibility));
				}
			}
		}
		
		return result;
	}



	void setWindowName(std::string name) {
		if(windowName != name) {
			windowName = std::move(name);
			if(opened) opened->window.setName(windowName.c_str());
		}
	}

	const std::string& getWindowName() const {
		return windowName;
	}


	void setSize(Math::Vec2i s) {
		if(size != s) {
			size = s;
			if(opened) {
				opened->window.setSize(size);
				owner.get().setVideoModeCompatibility(getVideoModeCompatibility()); //This will call reconfigure if resizeing is needed
			}
		}
	}

	Math::Vec2i getSize() const {
		return size;
	}

	void setSizeCallback(Window::SizeCallback cbk) {
		callbacks.sizeCbk = std::move(cbk);
	}

	const Window::SizeCallback& getSizeCallback() const {
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

	void setPositionCallback(Window::PositionCallback cbk) {
		callbacks.positionCbk = std::move(cbk);
	}

	const Window::PositionCallback& getPositionCallback() const {
		return callbacks.positionCbk;
	}


	void setState(Window::State st) {
		if(state != st) {
			state = st;
			if(opened) opened->window.setState(static_cast<GLFW::Window::State>(state));
		}
	}

	Window::State getState() const {
		return state;
	}

	void setStateCallback(Window::StateCallback cbk) {
		callbacks.stateCbk = std::move(cbk);
	}

	const Window::StateCallback& getStateCallback() const {
		return callbacks.stateCbk;
	}


	Math::Vec2f getScale() const {
		return opened ? opened->window.getScale() : Math::Vec2f(0.0f);
	}

	void setScaleCallback(Window::ScaleCallback cbk) {
		callbacks.scaleCbk = std::move(cbk);
	}

	const Window::ScaleCallback& getScaleCallback() const {
		return callbacks.scaleCbk;
	}


	void focus() {
		if(opened) opened->window.focus();
	}

	void setFocusCallback(Window::FocusCallback cbk) {
		callbacks.focusCbk = std::move(cbk);
	}

	const Window::FocusCallback& getFocusCallback() const {
		return callbacks.focusCbk;
	}


	bool shouldClose() const {
		return opened ? opened->window.shouldClose() : false;
	}

	void setShouldCloseCallback(Window::ShouldCloseCallback cbk) {
		callbacks.shouldCloseCbk = std::move(cbk);
	}

	const Window::ShouldCloseCallback& getShouldCloseCallback() const {
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

	void setMonitor(const Window::Monitor& mon) {
		monitor = getGLFWMonitor(mon);

		if(opened) {
			opened->window.setMonitor(monitor);
			size = opened->window.getSize();
			owner.get().setVideoModeCompatibility(getVideoModeCompatibility()); //This will call reconfigure if resizeing is needed
		}
	}
	
	Window::Monitor getMonitor() const {
		return constructMonitor(monitor);
	}



	Window::KeyboardEvent getKeyState(Window::KeyboardKey key) const {
		return opened 
		? static_cast<Window::KeyboardEvent>(opened->window.getKeyState(static_cast<GLFW::Window::KeyboardKey>(key)))
		: Window::KeyboardEvent::RELEASE;
	}

	void setKeyboardCallback(Window::KeyboardCallback cbk) {
		callbacks.keyboardCbk = std::move(cbk);
	}

	const Window::KeyboardCallback& getKeyboardCallback() const {
		return callbacks.keyboardCbk;
	}


	void setCharacterCallback(Window::CharacterCallback cbk) {
		callbacks.characterCbk = std::move(cbk);
	}

	const Window::CharacterCallback& getCharacterCallback() const {
		return callbacks.characterCbk;
	}


	Window::KeyboardEvent getMouseButtonState(Window::MouseButton but) const {
		return opened 
		? static_cast<Window::KeyboardEvent>(opened->window.getMouseButtonState(static_cast<GLFW::Window::MouseButton>(but)))
		: Window::KeyboardEvent::RELEASE;
	}

	void setMouseButtonCallback(Window::MouseButtonCallback cbk) {
		callbacks.mouseButtonCbk = std::move(cbk);
	}

	const Window::MouseButtonCallback& getMouseButtonCallback() const {
		return callbacks.mouseButtonCbk;
	}

	
	Math::Vec2d getMousePosition() const {
		return opened 
		? opened->window.getMousePosition()
		: Math::Vec2d();
	}

	void setMousePositionCallback(Window::MousePositionCallback cbk) {
		callbacks.mousePositionCbk = std::move(cbk);
	}

	const Window::MousePositionCallback& getMousePositionCallback() const {
		return callbacks.mousePositionCbk;
	}


	void setMouseScrollCallback(Window::MouseScrollCallback cbk) {
		callbacks.mouseScrollCbk = std::move(cbk);
	}

	const Window::MouseScrollCallback& getMouseScrollCallback() const {
		return callbacks.mouseScrollCbk;
	}


	void setCursorEnterCallback(Window::CursorEnterCallback cbk) {
		callbacks.cursorEnterCbk = std::move(cbk);
	}

	const Window::CursorEnterCallback& getCursorEnterCallback() const {
		return callbacks.cursorEnterCbk;
	}



	static Window::Monitor getPrimaryMonitor() {
		return WindowImpl::constructMonitor(GLFW::getGLFW().getPrimaryMonitor());
	}

	static std::vector<Window::Monitor> getMonitors() {
		const auto monitors = GLFW::getGLFW().getMonitors();
		std::vector<Window::Monitor> result;
		result.reserve(monitors.size());

		for(const auto& mon : monitors) {
			result.push_back(constructMonitor(mon));
		}

		return result;
	}

private:
	GLFW::Window::Callbacks createCallbacks() {
		return {
			std::bind(&WindowImpl::stateCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowImpl::positionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowImpl::sizeCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowImpl::resolutionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowImpl::scaleCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowImpl::shouldCloseCallback, std::ref(*this)),
			GLFW::Window::RefreshCallback(),
			std::bind(&WindowImpl::focusCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowImpl::keyboardCallback, std::ref(*this), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
			std::bind(&WindowImpl::characterCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowImpl::mouseButtonCallback, std::ref(*this), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
			std::bind(&WindowImpl::mousePositionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowImpl::mouseScrollCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowImpl::cursorEnterCallback, std::ref(*this), std::placeholders::_1)
		};
	}

	void resolutionCallback(Resolution) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());
		owner.get().setVideoModeCompatibility(getVideoModeCompatibility()); //This will call reconfigure if resizeing is needed
	}

	void sizeCallback(Math::Vec2i s) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		size = s;
		Utils::invokeIf(callbacks.sizeCbk, win, size);
	}

	void positionCallback(Math::Vec2i pos) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		position = pos;
		Utils::invokeIf(callbacks.positionCbk, win, position);
	}

	void stateCallback(GLFW::Window::State st) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		state = static_cast<Window::State>(st);
		Utils::invokeIf(callbacks.stateCbk, win, state);
	}

	void scaleCallback(Math::Vec2f sc) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.scaleCbk, win, sc);
	}

	void focusCallback(bool foc) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.focusCbk, win, foc);
	}

	void shouldCloseCallback() {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.shouldCloseCbk, win);
	}

	void keyboardCallback(GLFW::Window::KeyboardKey key, GLFW::Window::KeyboardEvent event, GLFW::Window::KeyboardModifiers mod) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(
			callbacks.keyboardCbk, 
			win, 
			static_cast<Window::KeyboardKey>(key),
			static_cast<Window::KeyboardEvent>(event),
			static_cast<Window::KeyboardModifiers>(mod)
		);
	}

	void characterCallback(uint character) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.characterCbk, win, character);
	}

	void mouseButtonCallback(GLFW::Window::MouseButton button, GLFW::Window::KeyboardEvent event, GLFW::Window::KeyboardModifiers mod) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(
			callbacks.mouseButtonCbk, 
			win, 
			static_cast<Window::MouseButton>(button),
			static_cast<Window::KeyboardEvent>(event),
			static_cast<Window::KeyboardModifiers>(mod)
		);
	}

	void mousePositionCallback(Math::Vec2d pos) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.mousePositionCbk, win, pos);
	}

	void mouseScrollCallback(Math::Vec2d deltaScroll) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.mouseScrollCbk, win, deltaScroll);
	}

	void cursorEnterCallback(bool entered) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.cursorEnterCbk, win, entered);
	}



	static std::tuple<vk::Extent2D, vk::Format, vk::ColorSpaceKHR, Graphics::OutputColorTransfer>
	convertParameters(	const Graphics::Vulkan& vulkan,
						const VideoMode& videoMode )
	{
		const auto frameDescriptor = videoMode.getFrameDescriptor();

		//Obtain the pixel format
		auto formats = Graphics::Frame::getPlaneDescriptors(frameDescriptor);
		assert(formats.size() == 1);

		auto& fmt = formats[0];
		std::tie(fmt.format, fmt.swizzle) = Graphics::optimizeFormat(std::make_tuple(fmt.format, fmt.swizzle));
		assert(fmt.swizzle == vk::ComponentMapping());

		//Obtain the color space
		const auto colorSpace = Graphics::toVulkan(
			videoMode.getColorPrimariesValue(), 
			videoMode.getColorTransferFunctionValue()
		);

		//Create the color transfer characteristics
		Graphics::OutputColorTransfer colorTransfer(frameDescriptor);

		constexpr vk::FormatFeatureFlags DESIRED_FLAGS = 
			vk::FormatFeatureFlagBits::eColorAttachment;
		const auto& supportedFormats = vulkan.listSupportedFormatsOptimal(DESIRED_FLAGS);
		colorTransfer.optimize(formats, supportedFormats);

		return std::make_tuple(fmt.extent, fmt.format, colorSpace, std::move(colorTransfer));
	}

	static Window::Monitor constructMonitor(GLFW::Monitor mon) {
		return Window::Monitor(Utils::Pimpl<Window::Monitor::Impl>({}, std::move(mon)));
	}

	static GLFW::Monitor getGLFWMonitor(const Window::Monitor& mon) {
		return mon.m_impl->monitor;
	}
};


/*
 * Window
 */

const Window::Monitor Window::NO_MONITOR = Window::Monitor();

Window::Window(	Instance& instance, 
				std::string name, 
				VideoMode videoMode,
				Math::Vec2i size,
				const Monitor& mon,
				Callbacks cbks )
	: Utils::Pimpl<WindowImpl>({}, *this, instance, size, mon, std::move(cbks))
	, ZuazoBase(
		instance, std::move(name), 
		PadRef((*this)->videoIn),
		std::bind(&WindowImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&WindowImpl::open, std::ref(**this), std::placeholders::_1),
		std::bind(&WindowImpl::close, std::ref(**this), std::placeholders::_1),
		std::bind(&WindowImpl::update, std::ref(**this)) )
	, VideoBase(
		std::move(videoMode),
		std::bind(&WindowImpl::setVideoMode, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
	, VideoScalerBase(
		std::bind(&WindowImpl::setScalingMode, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&WindowImpl::setScalingFilter, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
	, Signal::ConsumerLayout<Video>(makeProxy((*this)->videoIn))
{
	setVideoModeCompatibility((*this)->getVideoModeCompatibility());
}

Window::Window(Window&& other) = default;

Window::~Window() = default;

Window& Window::operator=(Window&& other) = default;


void Window::setWindowName(std::string name) {
	(*this)->setWindowName(std::move(name));
}
const std::string& Window::getWindowName() const {
	return (*this)->getWindowName();
}


void Window::setSize(Math::Vec2i size) {
	(*this)->setSize(size);
}

Math::Vec2i Window::getSize() const {
	return (*this)->getSize();
}

void Window::setSizeCallback(SizeCallback cbk) {
	(*this)->setSizeCallback(std::move(cbk));
}

const Window::SizeCallback& Window::getSizeCallback() const {
	return (*this)->getSizeCallback();
}


void Window::setPosition(Math::Vec2i pos) {
	(*this)->setPosition(pos);
}

Math::Vec2i Window::getPosition() const {
	return (*this)->getPosition();
}

void Window::setPositionCallback(PositionCallback cbk) {
	(*this)->setPositionCallback(std::move(cbk));
}

const Window::PositionCallback&	Window::getPositionCallback() const {
	return (*this)->getPositionCallback();
}


void Window::setState(State state) {
	(*this)->setState(state);
}

Window::State Window::getState() const {
	return (*this)->getState();
}

void Window::setStateCallback(StateCallback cbk) {
	(*this)->setStateCallback(std::move(cbk));
}

const Window::StateCallback& Window::getStateCallback() const {
	return (*this)->getStateCallback();
}


Math::Vec2f Window::getScale() const {
	return (*this)->getScale();
}

void Window::setScaleCallback(ScaleCallback cbk) {
	(*this)->setScaleCallback(std::move(cbk));
}

const Window::ScaleCallback& Window::getScaleCallback() const {
	return (*this)->getScaleCallback();
}


void Window::focus() {
	(*this)->focus();
}

void Window::setFocusCallback(FocusCallback cbk) {
	(*this)->setFocusCallback(std::move(cbk));
}

const Window::FocusCallback& Window::getFocusCallback() const {
	return (*this)->getFocusCallback();
}


bool Window::shouldClose() const {
	return (*this)->shouldClose();
}

void Window::setShouldCloseCallback(ShouldCloseCallback cbk) {
	(*this)->setShouldCloseCallback(std::move(cbk));
}

const Window::ShouldCloseCallback& Window::getShouldCloseCallback() const {
	return (*this)->getShouldCloseCallback();
}


void Window::setOpacity(float opa) {
	(*this)->setOpacity(opa);
}

float Window::getOpacity() const {
	return (*this)->getOpacity();
}


void Window::setResizeable(bool resizeable) {
	(*this)->setResizeable(resizeable);
}

bool Window::getResizeable() const {
	return (*this)->getResizeable();
}


void Window::setDecorated(bool deco) {
	(*this)->setDecorated(deco);
}

bool Window::getDecorated() const {
	return (*this)->getDecorated();
}


void Window::setMonitor(const Monitor& mon) {
	(*this)->setMonitor(mon);
}

Window::Monitor Window::getMonitor() const {
	return (*this)->getMonitor();
}



Window::KeyboardEvent Window::getKeyState(KeyboardKey key) const {
	return (*this)->getKeyState(key);
}

void Window::setKeyboardCallback(KeyboardCallback cbk) {
	(*this)->setKeyboardCallback(std::move(cbk));
}

const Window::KeyboardCallback& Window::getKeyboardCallback() const {
	return (*this)->getKeyboardCallback();
}


void Window::setCharacterCallback(CharacterCallback cbk) {
	(*this)->setCharacterCallback(std::move(cbk));
}

const Window::CharacterCallback& Window::getCharacterCallback() const {
	return (*this)->getCharacterCallback();
}


Window::KeyboardEvent Window::getMouseButtonState(MouseButton but) const {
	return (*this)->getMouseButtonState(but);
}

void Window::setMouseButtonCallback(MouseButtonCallback cbk) {
	(*this)->setMouseButtonCallback(std::move(cbk));
}

const Window::MouseButtonCallback& Window::getMouseButtonCallback() const {
	return (*this)->getMouseButtonCallback();
}


Math::Vec2d Window::getMousePosition() const {
	return (*this)->getMousePosition();
}

void Window::setMousePositionCallback(MousePositionCallback cbk) {
	(*this)->setMousePositionCallback(std::move(cbk));
}

const Window::MousePositionCallback& Window::getMousePositionCallback() const {
	return (*this)->getMousePositionCallback();
}


void Window::setMouseScrollCallback(MouseScrollCallback cbk) {
	(*this)->setMouseScrollCallback(std::move(cbk));
}

const Window::MouseScrollCallback& Window::getMouseScrollCallback() const {
	return (*this)->getMouseScrollCallback();
}


void Window::setCursorEnterCallback(CursorEnterCallback cbk) {
	(*this)->setCursorEnterCallback(std::move(cbk));
}

const Window::CursorEnterCallback& Window::getCursorEnterCallback() const {
	return (*this)->getCursorEnterCallback();
}



Window::Monitor Window::getPrimaryMonitor() {
	return WindowImpl::getPrimaryMonitor();
}

std::vector<Window::Monitor> Window::getMonitors() {
	return WindowImpl::getMonitors();
}

}

namespace Zuazo {

std::string_view toString(Consumers::Window::State state) {
	switch(state){

	ZUAZO_ENUM2STR_CASE( Consumers::Window::State, NORMAL)
	ZUAZO_ENUM2STR_CASE( Consumers::Window::State, HIDDEN)
	ZUAZO_ENUM2STR_CASE( Consumers::Window::State, FULLSCREEN)
	ZUAZO_ENUM2STR_CASE( Consumers::Window::State, ICONIFIED)
	ZUAZO_ENUM2STR_CASE( Consumers::Window::State, MAXIMIZED)

	default: return "";
	}
}

std::ostream& operator<<(std::ostream& os, Consumers::Window::State state) {
	return os << toString(state);
}


std::string_view toString(Consumers::Window::KeyboardKey key) {
	return GLFW::Window::getKeyName(static_cast<GLFW::Window::KeyboardKey>(key), 0);
}

std::ostream& operator<<(std::ostream& os, Consumers::Window::KeyboardKey key) {
	return os << toString(key);
}


std::string_view toString(Consumers::Window::KeyboardEvent event) {
	switch(event){

	ZUAZO_ENUM2STR_CASE( Consumers::Window::KeyboardEvent, RELEASE)
	ZUAZO_ENUM2STR_CASE( Consumers::Window::KeyboardEvent, PRESS)
	ZUAZO_ENUM2STR_CASE( Consumers::Window::KeyboardEvent, REPEAT)

	default: return "";
	}
}

std::ostream& operator<<(std::ostream& os, Consumers::Window::KeyboardEvent event) {
	return os << toString(event);
}


std::string toString(Consumers::Window::KeyboardModifiers mod) {
	std::stringstream ss;
	ss << mod;
	return ss.str();
}

std::ostream& operator<<(std::ostream& os, Consumers::Window::KeyboardModifiers mod) {
	uint32_t count = 0;
	if((mod & Consumers::Window::KeyboardModifiers::SHIFT) != Consumers::Window::KeyboardModifiers::NONE)		os << (count++ ? " | " : "") << "SHIFT";
	if((mod & Consumers::Window::KeyboardModifiers::CONTROL) != Consumers::Window::KeyboardModifiers::NONE)		os << (count++ ? " | " : "") << "CONTROL";
	if((mod & Consumers::Window::KeyboardModifiers::ALT) != Consumers::Window::KeyboardModifiers::NONE)			os << (count++ ? " | " : "") << "ALT";
	if((mod & Consumers::Window::KeyboardModifiers::SUPER) != Consumers::Window::KeyboardModifiers::NONE)		os << (count++ ? " | " : "") << "SUPER";
	if((mod & Consumers::Window::KeyboardModifiers::CAPS_LOCK) != Consumers::Window::KeyboardModifiers::NONE)	os << (count++ ? " | " : "") << "CAPS_LOCK";
	if((mod & Consumers::Window::KeyboardModifiers::NUM_LOCK) != Consumers::Window::KeyboardModifiers::NONE)	os << (count++ ? " | " : "") << "NUM_LOCK";
	return os;
}


std::string_view toString(Consumers::Window::MouseButton but) {
	switch(but) {
	case Consumers::Window::MouseButton::LEFT: return "LEFT";
	case Consumers::Window::MouseButton::RIGHT: return "RIGHT";
	case Consumers::Window::MouseButton::MIDDLE: return "MIDDLE";
	default: return "";
	}
}

std::ostream& operator<<(std::ostream& os, Consumers::Window::MouseButton but) {
	return os << toString(but);
}


}