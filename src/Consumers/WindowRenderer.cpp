#include <zuazo/Consumers/WindowRenderer.h>

#include "../GLFW.h"
#include "../GLFWConversions.h"

#include <zuazo/Graphics/Vulkan.h>
#include <zuazo/Graphics/VulkanConversions.h>
#include <zuazo/Graphics/ColorTransfer.h>
#include <zuazo/Graphics/StagedBuffer.h>
#include <zuazo/Graphics/DepthStencil.h>
#include <zuazo/Graphics/RenderPass.h>
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

struct WindowRenderer::Monitor::Impl {
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

WindowRenderer::Monitor::Monitor()
	: m_impl({})
{
}

WindowRenderer::Monitor::Monitor(Utils::Pimpl<Impl> pimpl)
	: m_impl(std::move(pimpl))
{
}

WindowRenderer::Monitor::Monitor(Monitor&& other) = default;

WindowRenderer::Monitor::~Monitor() = default;

WindowRenderer::Monitor& WindowRenderer::Monitor::operator=(Monitor&& other) = default;



std::string_view WindowRenderer::Monitor::getName() const {
	return m_impl->getName();
}

Math::Vec2d WindowRenderer::Monitor::getPhysicalSize() const {
	return m_impl->getPhysicalSize();
}

Math::Vec2i WindowRenderer::Monitor::getSize() const {
	return m_impl->getSize();
}

Math::Vec2i WindowRenderer::Monitor::getPosition() const {
	return m_impl->getPosition();
}

Rate WindowRenderer::Monitor::getFrameRate() const {
	return m_impl->getFrameRate();
}


/*
 * WindowRendererImpl
 */
struct WindowRendererImpl {
	struct Open {
		enum DescriptorLayouts {
			DESCRIPTOR_LAYOUT_WINDOW,

			DESCRIPTOR_LAYOUT_COUNT
		};

		const Graphics::Vulkan&						vulkan;

		GLFW::Window								window;
		vk::UniqueSurfaceKHR						surface;
		vk::UniqueCommandPool						commandPool;
		Graphics::CommandBuffer						commandBuffer;
		RendererBase::UniformBufferLayout			uniformBufferLayout;
		Graphics::StagedBuffer						uniformBuffer;
		vk::UniqueDescriptorPool					descriptorPool;
		vk::DescriptorSet							descriptorSet;
		vk::PipelineLayout							pipelineLayout;
		vk::UniqueSemaphore 						imageAvailableSemaphore;
		vk::UniqueSemaphore							renderFinishedSemaphore;
		vk::UniqueFence								renderFinishedFence;

		vk::Extent2D								extent;
		vk::Format 									colorFormat;
		vk::Format 									depthStencilFormat;
		vk::ColorSpaceKHR 							colorSpace;

		vk::UniqueSwapchainKHR						swapchain;
		std::vector<vk::UniqueImageView>			swapchainImageViews;
		std::unique_ptr<Graphics::DepthStencil>		depthStencilBuffer;
		Graphics::RenderPass						renderPass;
		std::vector<vk::UniqueFramebuffer>			framebuffers;

		Math::Mat4x4f&								uniformProjectionMatrix;
		Utils::BufferView<std::byte>				uniformOutputColorTransfer;
		Utils::Area									uniformFlushArea;
		vk::PipelineStageFlags						uniformFlushStages;


		Open(	const Graphics::Vulkan& vulkan,
				Math::Vec2i size,
				const char* name,
				GLFW::Monitor mon,
				GLFW::Window::Callbacks cbks,
				const WindowRenderer::Camera& camera ) 
			: vulkan(vulkan)
			, window(size, name, mon, std::move(cbks))
			, surface(window.getSurface(vulkan))
			, commandPool(createCommandPool(vulkan))
			, commandBuffer(createCommandBuffer(vulkan, *commandPool))
			, uniformBufferLayout(RendererBase::getUniformBufferLayout(vulkan))
			, uniformBuffer(createUniformBuffer(vulkan, uniformBufferLayout))
			, descriptorPool(createDescriptorPool(vulkan))
			, descriptorSet(createDescriptorSet(vulkan, *descriptorPool))
			, pipelineLayout(RendererBase::getPipelineLayout(vulkan))
			, imageAvailableSemaphore(vulkan.createSemaphore())
			, renderFinishedSemaphore(vulkan.createSemaphore())
			, renderFinishedFence(vulkan.createFence(true))

			, extent(Graphics::toVulkan(window.getResolution()))
			, colorFormat(vk::Format::eUndefined)
			, depthStencilFormat(vk::Format::eUndefined)
			, colorSpace(static_cast<vk::ColorSpaceKHR>(-1))

			, swapchain()
			, swapchainImageViews()
			, depthStencilBuffer()
			, renderPass()
			, framebuffers()

			, uniformProjectionMatrix(getProjectionMatrix(uniformBufferLayout, uniformBuffer))
			, uniformOutputColorTransfer(getOutputColorTransfer(uniformBufferLayout, uniformBuffer))
		{
			writeDescriptorSets();
			updateProjectionMatrixUniform(camera);
			//updateOutputColorTransferUniform(); //No data
		}

		~Open() {
			uniformBuffer.waitCompletion(vulkan);
			waitCompletion();
		}

		void recreate(	vk::Extent2D ext, 
						vk::Format colorFmt, 
						vk::Format depthStencilFmt,
						vk::ColorSpaceKHR cs, 
						const Graphics::OutputColorTransfer& ct,
						const WindowRenderer::Camera& cam ) 
		{
			enum {
				RECREATE_SWAPCHAIN,
				RECREATE_DEPTHBUFFER,
				RECREATE_RENDERPASS,
				RECREATE_FRAMEBUFFERS,
				UPDATE_PROJECTION_MATRIX,
				UPDATE_OUTPUT_COLOR_TRANSFER,

				MODIFICATION_COUNT
			};

			std::bitset<MODIFICATION_COUNT> modifications;

			if(extent != ext) {
				//Resolution has changed
				extent = ext;

				modifications.set(RECREATE_SWAPCHAIN);
				modifications.set(RECREATE_DEPTHBUFFER);
				modifications.set(UPDATE_PROJECTION_MATRIX);
			}

			if(colorFormat != colorFmt) {
				//Format has changed
				colorFormat = colorFmt;

				modifications.set(RECREATE_SWAPCHAIN);
				modifications.set(RECREATE_RENDERPASS);
			}

			if(depthStencilFormat != depthStencilFmt) {
				//Depth/Stencil format has changed
				depthStencilFormat = depthStencilFmt;

				modifications.set(RECREATE_DEPTHBUFFER);
				modifications.set(RECREATE_RENDERPASS);
			}

			if(colorSpace != cs) {
				//Color space has changed
				colorSpace = cs;

				modifications.set(RECREATE_SWAPCHAIN);
			}

			assert(uniformOutputColorTransfer.size() == ct.size());
			if(std::memcmp(uniformOutputColorTransfer.data(), ct.data(), ct.size())) {
				//Color transfer has changed
				modifications.set(UPDATE_OUTPUT_COLOR_TRANSFER);
			}

			//Recreate stuff accordingly
			if(modifications.any()) {
				//Wait until rendering finishes
				waitCompletion();

				if(modifications.test(RECREATE_SWAPCHAIN)) {
					const auto oldExtent = extent;

					if(extent != vk::Extent2D(0, 0) && colorFormat != vk::Format::eUndefined) {
						swapchain = createSwapchain(vulkan, *surface, extent, colorFormat, colorSpace, *swapchain);
						swapchainImageViews = createImageViews(vulkan, *swapchain, colorFormat);
					} else {
						swapchain.reset();
						swapchainImageViews.clear();
					}
					
					modifications.set(RECREATE_FRAMEBUFFERS);

					//Extent might have changed
					if(oldExtent != extent) {
						modifications.set(RECREATE_DEPTHBUFFER);
						modifications.set(UPDATE_PROJECTION_MATRIX);
					}
				} 

				if(modifications.test(RECREATE_DEPTHBUFFER)) {
					if(extent != vk::Extent2D(0, 0) && depthStencilFormat != vk::Format::eUndefined) {
						depthStencilBuffer = createDepthStencilBuffer(vulkan, extent, depthStencilFormat);
					} else {
						depthStencilBuffer.reset();
					}

					modifications.set(RECREATE_FRAMEBUFFERS);
				}

				if(modifications.test(RECREATE_RENDERPASS)) {
					if(colorFormat != vk::Format::eUndefined) {
						renderPass = createRenderPass(vulkan, colorFormat, depthStencilFormat);
					} else {
						renderPass = Graphics::RenderPass();
					}
					
					modifications.set(RECREATE_FRAMEBUFFERS);
				}

				if(modifications.test(RECREATE_FRAMEBUFFERS)) {
					if(renderPass.get() && swapchainImageViews.size()) {
						framebuffers = createFramebuffers(vulkan, renderPass.get(), swapchainImageViews, depthStencilBuffer.get(), extent);
					} else {
						framebuffers.clear();
					}
				}

				if(modifications.test(UPDATE_PROJECTION_MATRIX)) {
					updateProjectionMatrixUniform(cam);
				}
				
				if(modifications.test(UPDATE_OUTPUT_COLOR_TRANSFER)) {
					updateOutputColorTransferUniform(ct);
				} 
			}
		}

		void setCamera(const WindowRenderer::Camera& camera) {
			updateProjectionMatrixUniform(camera);
		}

		void draw(const RendererBase& renderer) {
			//Wait until any previous rendering has finished
			waitCompletion();

			//Acquire an image from the swapchain
			size_t index = acquireImage();

			if(index < framebuffers.size()) {
				const auto& frameBuffer = *(framebuffers[index]);

				//Begin writing to the command buffer. //TODO maybe reset pool?
				constexpr vk::CommandBufferBeginInfo cmdBegin(
					vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 
					nullptr
				);
				commandBuffer.begin(cmdBegin);

				//Begin a render pass
				const std::array clearValue = {
					vk::ClearValue(vk::ClearColorValue(std::array{ 0.0f, 0.0f, 0.0f, 0.0f }))
				};
				const vk::RenderPassBeginInfo rendBegin(
					renderPass.get(),													//Renderpass
					frameBuffer,														//Target framebuffer
					vk::Rect2D({0, 0}, extent),											//Extent
					clearValue.size(), clearValue.data()								//Attachment clear values
				);
				commandBuffer.beginRenderPass(rendBegin, vk::SubpassContents::eInline);


				//Evaluate if there are any layers
				if(!renderer.getLayers().empty()) {
					//Flush the unform buffer
					uniformBuffer.flushData(
						vulkan,
						uniformFlushArea,
						vulkan.getGraphicsQueueIndex(),
						vk::AccessFlagBits::eUniformRead,
						uniformFlushStages
					);
					uniformFlushArea = {};
					uniformFlushStages = {};

					//Set the dynamic viewport
					const std::array viewports = {
						vk::Viewport(
							0.0f, 0.0f,										//Origin
							static_cast<float>(extent.width), 				//Width
							static_cast<float>(extent.height),				//Height
							0.0f, 1.0f										//min, max depth
						),
					};
					commandBuffer.setViewport(0, viewports);

					//Set the dynamic scissor
					const std::array scissors = {
						vk::Rect2D(
							{ 0, 0 },										//Origin
							extent											//Size
						),
					};
					commandBuffer.setScissor(0, scissors);

					commandBuffer.bindDescriptorSets(
						vk::PipelineBindPoint::eGraphics,					//Pipeline bind point
						pipelineLayout,										//Pipeline layout
						DESCRIPTOR_LAYOUT_WINDOW,							//First index
						descriptorSet,										//Descriptor sets
						{}													//Dynamic offsets
					);

					//Draw all the layers
					renderer.draw(commandBuffer);
				}

				//End everything
				commandBuffer.endRenderPass();
				commandBuffer.end();

				//Send it to the queue
				const std::array imageAvailableSemaphores = {
					*imageAvailableSemaphore
				};
				const std::array renderFinishedSemaphores = {
					*renderFinishedSemaphore
				};
				const std::array commandBuffers = {
					commandBuffer.get()
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
		}

		void waitCompletion() {
			vulkan.waitForFences(*renderFinishedFence);
		}

	private:
		void updateProjectionMatrixUniform(const WindowRenderer::Camera& cam) {
			uniformBuffer.waitCompletion(vulkan);

			const auto size = Math::Vec2f(extent.width, extent.height);
			uniformProjectionMatrix = cam.calculateMatrix(size);

			uniformFlushArea |= uniformBufferLayout[RendererBase::DESCRIPTOR_BINDING_PROJECTION_MATRIX];
			uniformFlushStages |= vk::PipelineStageFlagBits::eVertexShader;
		}

		void updateOutputColorTransferUniform(const Graphics::OutputColorTransfer& outputColorTransfer) {
			uniformBuffer.waitCompletion(vulkan);
			
			assert(outputColorTransfer.size() == uniformOutputColorTransfer.size());
			std::memcpy(
				uniformOutputColorTransfer.data(),
				outputColorTransfer.data(),
				uniformOutputColorTransfer.size()
			);

			uniformFlushArea |= uniformBufferLayout[RendererBase::DESCRIPTOR_BINDING_OUTPUT_COLOR_TRANSFER];
			uniformFlushStages |= vk::PipelineStageFlagBits::eFragmentShader;
		}
	
		void writeDescriptorSets() {
			const std::array viewportBuffers = {
				vk::DescriptorBufferInfo(
					uniformBuffer.getBuffer(),															//Buffer
					uniformBufferLayout[RendererBase::DESCRIPTOR_BINDING_PROJECTION_MATRIX].offset(),	//Offset
					uniformBufferLayout[RendererBase::DESCRIPTOR_BINDING_PROJECTION_MATRIX].size()		//Size
				)
			};
			const std::array colorTransferBuffers = {
				vk::DescriptorBufferInfo(
					uniformBuffer.getBuffer(),																//Buffer
					uniformBufferLayout[RendererBase::DESCRIPTOR_BINDING_OUTPUT_COLOR_TRANSFER].offset(),	//Offset
					uniformBufferLayout[RendererBase::DESCRIPTOR_BINDING_OUTPUT_COLOR_TRANSFER].size()		//Size
				)
			};

			const std::array writeDescriptorSets = {
				vk::WriteDescriptorSet( //Viewport UBO
					descriptorSet,											//Descriptor set
					RendererBase::DESCRIPTOR_BINDING_PROJECTION_MATRIX,		//Binding
					0, 														//Index
					viewportBuffers.size(),									//Descriptor count		
					vk::DescriptorType::eUniformBuffer,						//Descriptor type
					nullptr, 												//Images 
					viewportBuffers.data(), 								//Buffers
					nullptr													//Texel buffers
				),
				vk::WriteDescriptorSet( //ColorTransfer UBO
					descriptorSet,											//Descriptor set
					RendererBase::DESCRIPTOR_BINDING_OUTPUT_COLOR_TRANSFER,	//Binding
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

		size_t acquireImage() {
			uint32_t index;
			const vk::Result result = vulkan.getDevice().acquireNextImageKHR(
				*swapchain, 						
				Graphics::Vulkan::NO_TIMEOUT,
				*imageAvailableSemaphore,
				nullptr,
				&index,
				vulkan.getDispatcher()
			);

			return (result == vk::Result::eSuccess) || (result == vk::Result::eSuboptimalKHR) ? index : framebuffers.size();
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

		static Graphics::CommandBuffer createCommandBuffer(	const Graphics::Vulkan& vulkan,
															vk::CommandPool pool )
		{
			return Graphics::CommandBuffer(
				vulkan,
				vulkan.allocateCommnadBuffer(pool, vk::CommandBufferLevel::ePrimary)
			);
		}

		static Graphics::StagedBuffer createUniformBuffer(	const Graphics::Vulkan& vulkan, 
															const RendererBase::UniformBufferLayout& layout ) 
		{
			return Graphics::StagedBuffer(
				vulkan,
				vk::BufferUsageFlagBits::eUniformBuffer,
				layout.back().end()
			);
		}

		static vk::UniqueDescriptorPool createDescriptorPool(const Graphics::Vulkan& vulkan){
			const std::array poolSizes = {
				vk::DescriptorPoolSize(
					vk::DescriptorType::eUniformBuffer,					//Descriptor type
					RendererBase::DESCRIPTOR_COUNT						//Descriptor count
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
			const auto layout = RendererBase::getDescriptorSetLayout(vulkan);
			return vulkan.allocateDescriptorSet(pool, layout).release();
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
			const auto sharingMode = (queueFamilies.size() > 1) ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
			
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

		static std::unique_ptr<Graphics::DepthStencil> createDepthStencilBuffer(	const Graphics::Vulkan& vulkan,
																					vk::Extent2D extent,
																					vk::Format format )
		{
			return Utils::makeUnique<Graphics::DepthStencil>(vulkan, extent, format);
		}

		static Graphics::RenderPass createRenderPass(	const Graphics::Vulkan& vulkan, 
														vk::Format colorFormat,
														vk::Format depthStencilFormat )
		{
			const std::array planeDescriptors = {
				Graphics::Image::PlaneDescriptor{
					vk::Extent2D(),
					colorFormat,
					vk::ComponentMapping()
				}
			};

			return Graphics::RenderPass(
				vulkan,
				planeDescriptors,
				Graphics::fromVulkanDepthStencil(depthStencilFormat),
				vk::ImageLayout::ePresentSrcKHR
			);
		}

		static std::vector<vk::UniqueFramebuffer> createFramebuffers(	const Graphics::Vulkan& vulkan,
																		vk::RenderPass renderPass,
																		const std::vector<vk::UniqueImageView>& imageViews,
																		const Graphics::DepthStencil* depthBuffer,
																		vk::Extent2D extent )
		{
			std::vector<vk::UniqueFramebuffer> result(imageViews.size());

			for(size_t i = 0; i < result.size(); i++){
				const std::array attachments = {
					*imageViews[i],
					depthBuffer ? depthBuffer->getImageView() : vk::ImageView()
				};

				const vk::FramebufferCreateInfo createInfo(
					{},
					renderPass,
					depthBuffer ? attachments.size() : attachments.size() - 1,
					attachments.data(),
					extent.width, extent.height,
					1
				);

				result[i] = vulkan.createFramebuffer(createInfo);
			}

			return result;
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

		static Math::Mat4x4f& getProjectionMatrix(	const RendererBase::UniformBufferLayout& uniformBufferLayout,
													Graphics::StagedBuffer& uniformBuffer )
		{
			const auto& area = uniformBufferLayout[RendererBase::DESCRIPTOR_BINDING_PROJECTION_MATRIX];
			return *(reinterpret_cast<Math::Mat4x4f*>(area.begin(uniformBuffer.data())));
		}

		static Utils::BufferView<std::byte> getOutputColorTransfer(	const RendererBase::UniformBufferLayout& uniformBufferLayout,
																	Graphics::StagedBuffer& uniformBuffer )
		{
			const auto& area = uniformBufferLayout[RendererBase::DESCRIPTOR_BINDING_OUTPUT_COLOR_TRANSFER];
			return Utils::BufferView<std::byte>(area.begin(uniformBuffer.data()), area.size());
		}
	};

	std::reference_wrapper<WindowRenderer>		owner;

	std::string									windowName;
	Math::Vec2i 								size;
	Math::Vec2i 								position;
	WindowRenderer::State 						state;
	float										opacity;
	bool										resizeable;
	bool										decorated;
	GLFW::Monitor								monitor;

	WindowRenderer::Callbacks					callbacks;
	
	std::unique_ptr<Open>						opened;
	bool										hasChanged;


	static constexpr auto PRIORITY = Instance::OUTPUT_PRIORITY;
	static constexpr auto NO_POSTION = Math::Vec2i(std::numeric_limits<int32_t>::min());

	WindowRendererImpl(	WindowRenderer& owner,
				Instance& instance,
				Math::Vec2i size,
				const WindowRenderer::Monitor& mon,
				WindowRenderer::Callbacks callbacks)
		: owner(owner)
		, windowName(instance.getApplicationInfo().getName())
		, size(size)
		, position(NO_POSTION)
		, state(WindowRenderer::State::NORMAL)
		, opacity(1.0f)
		, resizeable(true)
		, decorated(true)
		, monitor(getGLFWMonitor(mon))
		, callbacks(std::move(callbacks))
	{
	}
	~WindowRendererImpl() = default;


	void moved(ZuazoBase& base) {
		owner = static_cast<WindowRenderer&>(base);
	}

	void open(ZuazoBase& base) {
		assert(!opened);
		WindowRenderer& window = static_cast<WindowRenderer&>(base);
		assert(&owner.get() == &window);

		//Try to open it
		opened = std::make_unique<Open>(
			window.getInstance().getVulkan(),
			size,
			windowName.c_str(),
			monitor,
			createCallbacks(),
			window.getCamera()
		);
		
		//Set everything as desired
		//opened->window.setName(windowName); //Already set when constructing
		//opened->window.setSize(size); //Already set when constructing
		if(position != NO_POSTION) opened->window.setPosition(position);
		opened->window.setState(static_cast<GLFW::Window::State>(state));
		opened->window.setOpacity(opacity);
		opened->window.setResizeable(resizeable);
		opened->window.setDecorated(decorated);

		window.setVideoModeCompatibility(getVideoModeCompatibility());

		//Timing will be enabled on the setVideoMode() callback
		//win.enablePeriodicUpdate(PRIORITY, getPeriod(videoMode.getFrameRateValue())); 

		hasChanged = true;
	}

	void close(ZuazoBase& base) {
		assert(opened);
		WindowRenderer& window = static_cast<WindowRenderer&>(base);
		assert(&owner.get() == &window);

		window.disablePeriodicUpdate();
		opened.reset();
		window.setVideoModeCompatibility(getVideoModeCompatibility());
	}

	void setVideoMode(VideoBase& base, const VideoMode& videoMode) {
		auto& window = static_cast<WindowRenderer&>(base);
		recreate(window, videoMode, window.getDepthStencilFormat());
	}

	void setDepthStencilFormat(RendererBase& base, const Utils::Limit<DepthStencilFormat>& depthStencil) {
		auto& window = static_cast<WindowRenderer&>(base);
		recreate(window, window.getVideoMode(), depthStencil);
	}

	void setCamera(RendererBase& base, const RendererBase::Camera& camera) {
		WindowRenderer& window = static_cast<WindowRenderer&>(base);
		assert(&owner.get() == &window);

		if(opened) {
			opened->setCamera(camera);
			hasChanged = true;
		}
	}

	Graphics::RenderPass getRenderPass(const RendererBase& base) {
		(void)(base);
		return opened ? opened->renderPass : Graphics::RenderPass();
	}

	void update() {
		assert(opened);
		const auto& window = owner.get();

		if(hasChanged || window.layersHaveChanged()) {
			opened->draw(window);

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
			WindowRenderer& win = owner.get();
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

	Utils::Limit<DepthStencilFormat> getDepthStencilFormatCompatibility() {
		Utils::Discrete<DepthStencilFormat> result;
		const auto& vulkan = owner.get().getInstance().getVulkan();
		constexpr vk::FormatFeatureFlags DESIRED_FLAGS = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
		const auto& support = vulkan.listSupportedFormatsOptimal(DESIRED_FLAGS);
		assert(std::is_sorted(support.cbegin(), support.cend())); //For binary search

		//Having no depth/stencil is supported:
		result.emplace_back(DepthStencilFormat::NONE);

		//Test for each format
		for(auto i = Utils::EnumTraits<DepthStencilFormat>::first(); i <= Utils::EnumTraits<DepthStencilFormat>::last(); ++i) {
			//Convert it into a Vulkan format
			const auto conversion = Graphics::toVulkan(i);

			//Check if it is supported
			const auto supported = std::binary_search(
				support.cbegin(), support.cend(),
				conversion
			);

			if(supported) {
				result.push_back(i);
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

	void setSizeCallback(WindowRenderer::SizeCallback cbk) {
		callbacks.sizeCbk = std::move(cbk);
	}

	const WindowRenderer::SizeCallback& getSizeCallback() const {
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

	void setPositionCallback(WindowRenderer::PositionCallback cbk) {
		callbacks.positionCbk = std::move(cbk);
	}

	const WindowRenderer::PositionCallback& getPositionCallback() const {
		return callbacks.positionCbk;
	}


	void setState(WindowRenderer::State st) {
		if(state != st) {
			state = st;
			if(opened) opened->window.setState(static_cast<GLFW::Window::State>(state));
		}
	}

	WindowRenderer::State getState() const {
		return state;
	}

	void setStateCallback(WindowRenderer::StateCallback cbk) {
		callbacks.stateCbk = std::move(cbk);
	}

	const WindowRenderer::StateCallback& getStateCallback() const {
		return callbacks.stateCbk;
	}


	Math::Vec2f getScale() const {
		return opened ? opened->window.getScale() : Math::Vec2f(0.0f);
	}

	void setScaleCallback(WindowRenderer::ScaleCallback cbk) {
		callbacks.scaleCbk = std::move(cbk);
	}

	const WindowRenderer::ScaleCallback& getScaleCallback() const {
		return callbacks.scaleCbk;
	}


	void focus() {
		if(opened) opened->window.focus();
	}

	void setFocusCallback(WindowRenderer::FocusCallback cbk) {
		callbacks.focusCbk = std::move(cbk);
	}

	const WindowRenderer::FocusCallback& getFocusCallback() const {
		return callbacks.focusCbk;
	}


	bool shouldClose() const {
		return opened ? opened->window.shouldClose() : false;
	}

	void setShouldCloseCallback(WindowRenderer::ShouldCloseCallback cbk) {
		callbacks.shouldCloseCbk = std::move(cbk);
	}

	const WindowRenderer::ShouldCloseCallback& getShouldCloseCallback() const {
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

	void setMonitor(const WindowRenderer::Monitor& mon) {
		monitor = getGLFWMonitor(mon);

		if(opened) {
			opened->window.setMonitor(monitor);
			size = opened->window.getSize();
			owner.get().setVideoModeCompatibility(getVideoModeCompatibility()); //This will call reconfigure if resizeing is needed
		}
	}
	
	WindowRenderer::Monitor getMonitor() const {
		return constructMonitor(monitor);
	}



	KeyEvent getKeyState(KeyboardKey key) const {
		return opened 
		? fromGLFW(opened->window.getKeyState(toGLFW(key))) 
		: KeyEvent::RELEASE;
	}

	void setKeyboardCallback(WindowRenderer::KeyboardCallback cbk) {
		callbacks.keyboardCbk = std::move(cbk);
	}

	const WindowRenderer::KeyboardCallback& getKeyboardCallback() const {
		return callbacks.keyboardCbk;
	}


	void setCharacterCallback(WindowRenderer::CharacterCallback cbk) {
		callbacks.characterCbk = std::move(cbk);
	}

	const WindowRenderer::CharacterCallback& getCharacterCallback() const {
		return callbacks.characterCbk;
	}


	KeyEvent getMouseButtonState(MouseKey but) const {
		return opened 
		? fromGLFW(opened->window.getMouseButtonState(toGLFW(but)))
		: KeyEvent::RELEASE;
	}

	void setMouseButtonCallback(WindowRenderer::MouseButtonCallback cbk) {
		callbacks.mouseButtonCbk = std::move(cbk);
	}

	const WindowRenderer::MouseButtonCallback& getMouseButtonCallback() const {
		return callbacks.mouseButtonCbk;
	}

	
	Math::Vec2d getMousePosition() const {
		return opened 
		? opened->window.getMousePosition()
		: Math::Vec2d();
	}

	void setMousePositionCallback(WindowRenderer::MousePositionCallback cbk) {
		callbacks.mousePositionCbk = std::move(cbk);
	}

	const WindowRenderer::MousePositionCallback& getMousePositionCallback() const {
		return callbacks.mousePositionCbk;
	}


	void setMouseScrollCallback(WindowRenderer::MouseScrollCallback cbk) {
		callbacks.mouseScrollCbk = std::move(cbk);
	}

	const WindowRenderer::MouseScrollCallback& getMouseScrollCallback() const {
		return callbacks.mouseScrollCbk;
	}


	void setCursorEnterCallback(WindowRenderer::CursorEnterCallback cbk) {
		callbacks.cursorEnterCbk = std::move(cbk);
	}

	const WindowRenderer::CursorEnterCallback& getCursorEnterCallback() const {
		return callbacks.cursorEnterCbk;
	}



	static WindowRenderer::Monitor getPrimaryMonitor() {
		return WindowRendererImpl::constructMonitor(GLFW::getGLFW().getPrimaryMonitor());
	}

	static std::vector<WindowRenderer::Monitor> getMonitors() {
		const auto monitors = GLFW::getGLFW().getMonitors();
		std::vector<WindowRenderer::Monitor> result;
		result.reserve(monitors.size());

		for(const auto& mon : monitors) {
			result.push_back(constructMonitor(mon));
		}

		return result;
	}

private:
	void recreate(	WindowRenderer& window, 
					const VideoMode& videoMode, 
					const Utils::Limit<DepthStencilFormat>& depthStencil )
	{
		assert(&owner.get() == &window);

		if(opened) {
			window.disablePeriodicUpdate();

			if(videoMode && depthStencil) {
				const auto frameDesc = videoMode.getFrameDescriptor();
				const auto depthStencilFormat = Graphics::toVulkan(depthStencil.value());
				const auto [extent, colorformat, colorSpace, colorTransfer] = convertParameters(window.getInstance().getVulkan(), frameDesc);
				const auto framePeriod = getPeriod(videoMode.getFrameRateValue());

				//Update the parameters
				opened->recreate(
					extent, 
					colorformat, 
					depthStencilFormat, 
					colorSpace, 
					colorTransfer, 
					window.getCamera()
				);
				window.enablePeriodicUpdate(PRIORITY, framePeriod);
			} else {
				//Unset the stuff
				opened->recreate(
					vk::Extent2D(0, 0), 
					vk::Format::eUndefined, 
					vk::Format::eUndefined, 
					static_cast<vk::ColorSpaceKHR>(-1), 
					Graphics::OutputColorTransfer(), 
					window.getCamera()
				);
			}

			hasChanged = true;
		}
	}

	GLFW::Window::Callbacks createCallbacks() {
		return {
			std::bind(&WindowRendererImpl::stateCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowRendererImpl::positionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowRendererImpl::sizeCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowRendererImpl::resolutionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowRendererImpl::scaleCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowRendererImpl::shouldCloseCallback, std::ref(*this)),
			GLFW::Window::RefreshCallback(),
			std::bind(&WindowRendererImpl::focusCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowRendererImpl::keyboardCallback, std::ref(*this), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
			std::bind(&WindowRendererImpl::characterCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowRendererImpl::mouseButtonCallback, std::ref(*this), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
			std::bind(&WindowRendererImpl::mousePositionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowRendererImpl::mouseScrollCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&WindowRendererImpl::cursorEnterCallback, std::ref(*this), std::placeholders::_1)
		};
	}

	void resolutionCallback(Resolution) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());
		owner.get().setVideoModeCompatibility(getVideoModeCompatibility()); //This will call reconfigure if resizeing is needed
	}

	void sizeCallback(Math::Vec2i s) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		size = s;
		Utils::invokeIf(callbacks.sizeCbk, win, size);
	}

	void positionCallback(Math::Vec2i pos) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		position = pos;
		Utils::invokeIf(callbacks.positionCbk, win, position);
	}

	void stateCallback(GLFW::Window::State st) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		state = static_cast<WindowRenderer::State>(st);
		Utils::invokeIf(callbacks.stateCbk, win, state);
	}

	void scaleCallback(Math::Vec2f sc) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.scaleCbk, win, sc);
	}

	void focusCallback(bool foc) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.focusCbk, win, foc);
	}

	void shouldCloseCallback() {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.shouldCloseCbk, win);
	}

	void keyboardCallback(GLFW::Window::KeyboardKey key, GLFW::Window::KeyboardEvent event, GLFW::Window::KeyboardModifiers mod) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(
			callbacks.keyboardCbk, 
			win, 
			fromGLFW(key),
			fromGLFW(event),
			fromGLFW(mod)
		);
	}

	void characterCallback(uint character) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.characterCbk, win, character);
	}

	void mouseButtonCallback(GLFW::Window::MouseButton button, GLFW::Window::KeyboardEvent event, GLFW::Window::KeyboardModifiers mod) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(
			callbacks.mouseButtonCbk, 
			win, 
			fromGLFW(button),
			fromGLFW(event),
			fromGLFW(mod)
		);
	}

	void mousePositionCallback(Math::Vec2d pos) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.mousePositionCbk, win, pos);
	}

	void mouseScrollCallback(Math::Vec2d deltaScroll) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.mouseScrollCbk, win, deltaScroll);
	}

	void cursorEnterCallback(bool entered) {
		assert(opened);
		auto& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.cursorEnterCbk, win, entered);
	}



	static std::tuple<vk::Extent2D, vk::Format, vk::ColorSpaceKHR, Graphics::OutputColorTransfer>
	convertParameters(	const Graphics::Vulkan& vulkan,
						const Graphics::Frame::Descriptor& frameDescriptor )
	{
		//Obtain the pixel format
		auto formats = Graphics::Frame::getPlaneDescriptors(frameDescriptor);
		assert(formats.size() == 1);

		auto& fmt = formats[0];
		std::tie(fmt.format, fmt.swizzle) = Graphics::optimizeFormat(std::make_tuple(fmt.format, fmt.swizzle));
		assert(fmt.swizzle == vk::ComponentMapping());

		//Obtain the color space
		const auto colorSpace = Graphics::toVulkan(
			frameDescriptor.getColorPrimaries(), 
			frameDescriptor.getColorTransferFunction()
		);

		//Create the color transfer characteristics
		Graphics::OutputColorTransfer colorTransfer(frameDescriptor);

		constexpr vk::FormatFeatureFlags DESIRED_FLAGS = 
			vk::FormatFeatureFlagBits::eColorAttachment;
		const auto& supportedFormats = vulkan.listSupportedFormatsOptimal(DESIRED_FLAGS);
		colorTransfer.optimize(formats, supportedFormats);

		return std::make_tuple(
			fmt.extent, 
			fmt.format, 
			colorSpace, 
			std::move(colorTransfer)
		);
	}

	static WindowRenderer::Monitor constructMonitor(GLFW::Monitor mon) {
		return WindowRenderer::Monitor(Utils::Pimpl<WindowRenderer::Monitor::Impl>({}, std::move(mon)));
	}

	static GLFW::Monitor getGLFWMonitor(const WindowRenderer::Monitor& mon) {
		return mon.m_impl->monitor;
	}
};


/*
 * Window
 */

const WindowRenderer::Monitor WindowRenderer::NO_MONITOR = WindowRenderer::Monitor();

WindowRenderer::WindowRenderer(	Instance& instance, 
								std::string name, 
								VideoMode videoMode,
								Utils::Limit<DepthStencilFormat> depthStencil,
								Math::Vec2i size,
								const Monitor& mon,
								Callbacks cbks )
	: Utils::Pimpl<WindowRendererImpl>({}, *this, instance, size, mon, std::move(cbks))
	, ZuazoBase(
		instance, std::move(name), 
		{},
		std::bind(&WindowRendererImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&WindowRendererImpl::open, std::ref(**this), std::placeholders::_1),
		std::bind(&WindowRendererImpl::close, std::ref(**this), std::placeholders::_1),
		std::bind(&WindowRendererImpl::update, std::ref(**this)) )
	, VideoBase(
		std::move(videoMode),
		std::bind(&WindowRendererImpl::setVideoMode, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
	, RendererBase(
		std::move(depthStencil),
		std::bind(&WindowRendererImpl::setDepthStencilFormat, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&WindowRendererImpl::setCamera, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&WindowRendererImpl::getRenderPass, std::ref(**this), std::placeholders::_1)
	)
{
	setVideoModeCompatibility((*this)->getVideoModeCompatibility());
	setDepthStencilFormatCompatibility((*this)->getDepthStencilFormatCompatibility());
}

WindowRenderer::WindowRenderer(WindowRenderer&& other) = default;

WindowRenderer::~WindowRenderer() = default;

WindowRenderer& WindowRenderer::operator=(WindowRenderer&& other) = default;


void WindowRenderer::setWindowName(std::string name) {
	(*this)->setWindowName(std::move(name));
}
const std::string& WindowRenderer::getWindowName() const {
	return (*this)->getWindowName();
}


void WindowRenderer::setSize(Math::Vec2i size) {
	(*this)->setSize(size);
}

Math::Vec2i WindowRenderer::getSize() const {
	return (*this)->getSize();
}

void WindowRenderer::setSizeCallback(SizeCallback cbk) {
	(*this)->setSizeCallback(std::move(cbk));
}

const WindowRenderer::SizeCallback& WindowRenderer::getSizeCallback() const {
	return (*this)->getSizeCallback();
}


void WindowRenderer::setPosition(Math::Vec2i pos) {
	(*this)->setPosition(pos);
}

Math::Vec2i WindowRenderer::getPosition() const {
	return (*this)->getPosition();
}

void WindowRenderer::setPositionCallback(PositionCallback cbk) {
	(*this)->setPositionCallback(std::move(cbk));
}

const WindowRenderer::PositionCallback&	WindowRenderer::getPositionCallback() const {
	return (*this)->getPositionCallback();
}


void WindowRenderer::setState(State state) {
	(*this)->setState(state);
}

WindowRenderer::State WindowRenderer::getState() const {
	return (*this)->getState();
}

void WindowRenderer::setStateCallback(StateCallback cbk) {
	(*this)->setStateCallback(std::move(cbk));
}

const WindowRenderer::StateCallback& WindowRenderer::getStateCallback() const {
	return (*this)->getStateCallback();
}


Math::Vec2f WindowRenderer::getScale() const {
	return (*this)->getScale();
}

void WindowRenderer::setScaleCallback(ScaleCallback cbk) {
	(*this)->setScaleCallback(std::move(cbk));
}

const WindowRenderer::ScaleCallback& WindowRenderer::getScaleCallback() const {
	return (*this)->getScaleCallback();
}


void WindowRenderer::focus() {
	(*this)->focus();
}

void WindowRenderer::setFocusCallback(FocusCallback cbk) {
	(*this)->setFocusCallback(std::move(cbk));
}

const WindowRenderer::FocusCallback& WindowRenderer::getFocusCallback() const {
	return (*this)->getFocusCallback();
}


bool WindowRenderer::shouldClose() const {
	return (*this)->shouldClose();
}

void WindowRenderer::setShouldCloseCallback(ShouldCloseCallback cbk) {
	(*this)->setShouldCloseCallback(std::move(cbk));
}

const WindowRenderer::ShouldCloseCallback& WindowRenderer::getShouldCloseCallback() const {
	return (*this)->getShouldCloseCallback();
}


void WindowRenderer::setOpacity(float opa) {
	(*this)->setOpacity(opa);
}

float WindowRenderer::getOpacity() const {
	return (*this)->getOpacity();
}


void WindowRenderer::setResizeable(bool resizeable) {
	(*this)->setResizeable(resizeable);
}

bool WindowRenderer::getResizeable() const {
	return (*this)->getResizeable();
}


void WindowRenderer::setDecorated(bool deco) {
	(*this)->setDecorated(deco);
}

bool WindowRenderer::getDecorated() const {
	return (*this)->getDecorated();
}


void WindowRenderer::setMonitor(const Monitor& mon) {
	(*this)->setMonitor(mon);
}

WindowRenderer::Monitor WindowRenderer::getMonitor() const {
	return (*this)->getMonitor();
}



KeyEvent WindowRenderer::getKeyState(KeyboardKey key) const {
	return (*this)->getKeyState(key);
}

void WindowRenderer::setKeyboardCallback(KeyboardCallback cbk) {
	(*this)->setKeyboardCallback(std::move(cbk));
}

const WindowRenderer::KeyboardCallback& WindowRenderer::getKeyboardCallback() const {
	return (*this)->getKeyboardCallback();
}


void WindowRenderer::setCharacterCallback(CharacterCallback cbk) {
	(*this)->setCharacterCallback(std::move(cbk));
}

const WindowRenderer::CharacterCallback& WindowRenderer::getCharacterCallback() const {
	return (*this)->getCharacterCallback();
}


KeyEvent WindowRenderer::getMouseButtonState(MouseKey but) const {
	return (*this)->getMouseButtonState(but);
}

void WindowRenderer::setMouseButtonCallback(MouseButtonCallback cbk) {
	(*this)->setMouseButtonCallback(std::move(cbk));
}

const WindowRenderer::MouseButtonCallback& WindowRenderer::getMouseButtonCallback() const {
	return (*this)->getMouseButtonCallback();
}


Math::Vec2d WindowRenderer::getMousePosition() const {
	return (*this)->getMousePosition();
}

void WindowRenderer::setMousePositionCallback(MousePositionCallback cbk) {
	(*this)->setMousePositionCallback(std::move(cbk));
}

const WindowRenderer::MousePositionCallback& WindowRenderer::getMousePositionCallback() const {
	return (*this)->getMousePositionCallback();
}


void WindowRenderer::setMouseScrollCallback(MouseScrollCallback cbk) {
	(*this)->setMouseScrollCallback(std::move(cbk));
}

const WindowRenderer::MouseScrollCallback& WindowRenderer::getMouseScrollCallback() const {
	return (*this)->getMouseScrollCallback();
}


void WindowRenderer::setCursorEnterCallback(CursorEnterCallback cbk) {
	(*this)->setCursorEnterCallback(std::move(cbk));
}

const WindowRenderer::CursorEnterCallback& WindowRenderer::getCursorEnterCallback() const {
	return (*this)->getCursorEnterCallback();
}



WindowRenderer::Monitor WindowRenderer::getPrimaryMonitor() {
	return WindowRendererImpl::getPrimaryMonitor();
}

std::vector<WindowRenderer::Monitor> WindowRenderer::getMonitors() {
	return WindowRendererImpl::getMonitors();
}

}