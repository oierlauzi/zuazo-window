#include <zuazo/Renderers/Window.h>

#include "../GLFW/Window.h"
#include "../GLFWConversions.h"

#include <zuazo/Graphics/Vulkan.h>
#include <zuazo/Graphics/VulkanConversions.h>
#include <zuazo/Graphics/ColorTransfer.h>
#include <zuazo/Graphics/StagedBuffer.h>
#include <zuazo/Graphics/RenderPass.h>
#include <zuazo/Graphics/UniformBuffer.h>
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
#include <unordered_map>

namespace Zuazo::Renderers {

/*
 * WindowImpl
 */

struct WindowImpl {
	struct Open {
		Instance& 									instance;
		const Graphics::Vulkan&						vulkan;

		GLFW::Window								window;
		vk::UniqueSurfaceKHR						surface;
		vk::UniqueCommandPool						commandPool;
		Graphics::CommandBuffer						commandBuffer;
		vk::UniqueDescriptorPool					descriptorPool;
		vk::DescriptorSet							uniformDescriptorSet;
		vk::PipelineLayout							pipelineLayout;
		vk::UniqueSemaphore 						imageAvailableSemaphore;
		vk::UniqueSemaphore							renderFinishedSemaphore;
		vk::UniqueFence								renderFinishedFence;

		vk::Extent2D								extent;
		vk::Format									colorFormat;
		vk::ColorSpaceKHR 							colorSpace;
		Graphics::ColorTransferWrite				colorTransfer;
		DepthStencilFormat							depthStencilFormat;

		vk::UniqueSwapchainKHR						swapchain;
		std::vector<Graphics::Image>				swapchainImages;
		Graphics::RenderPass						renderPass;
		std::vector<vk::UniqueFramebuffer>			framebuffers;
		Utils::BufferView<const vk::ClearValue>		clearValues;
		Graphics::UniformBuffer						uniformBuffer;


		Open(	Instance& instance,
				Math::Vec2i size,
				const std::string& title,
				Window::Monitor monitor,
				WindowImpl& impl,
				const Window::Camera& camera ) 
			: instance(instance)
			, vulkan(instance.getVulkan())
			, window(createWindow(size, title, monitor, impl))
			, surface(createSurface(vulkan, window))
			, commandPool(createCommandPool(vulkan))
			, commandBuffer(createCommandBuffer(vulkan, *commandPool))
			, descriptorPool(createDescriptorPool(vulkan))
			, uniformDescriptorSet(createUniformDescriptorSet(vulkan, *descriptorPool))
			, pipelineLayout(RendererBase::getBasePipelineLayout(vulkan))
			, imageAvailableSemaphore(vulkan.createSemaphore())
			, renderFinishedSemaphore(vulkan.createSemaphore())
			, renderFinishedFence(vulkan.createFence(true))

			, extent(Graphics::toVulkan(window.getResolution()))
			, colorFormat(vk::Format::eUndefined)
			, colorSpace(static_cast<vk::ColorSpaceKHR>(-1))
			, colorTransfer()
			, depthStencilFormat(DepthStencilFormat::NONE)
			
			, swapchain()
			, swapchainImages()
			, renderPass()
			, framebuffers()
			, clearValues(Graphics::RenderPass::getClearValues(depthStencilFormat))
			, uniformBuffer(vulkan, RendererBase::getUniformBufferSizes())
		{
			uniformBuffer.writeDescirptorSet(vulkan, uniformDescriptorSet);
			updateProjectionMatrixUniform(camera);
		}

		~Open() {
			uniformBuffer.waitCompletion(vulkan);
			waitCompletion();

			//Ensure that there are no pending events
			const auto emitterId = getEmitterId(getUserPointer(window));
			window = GLFW::Window(); //After this line no more events will be emitted
			instance.removeEvent(emitterId); //Clean all pending events
		}

		void recreate(	vk::Extent2D ext,
						vk::Format colorFmt,
						vk::ColorSpaceKHR cs,
						Graphics::ColorTransferWrite ct,
						DepthStencilFormat depthStencilFmt,
						const Window::Camera& cam ) 
		{
			enum {
				RECREATE_SWAPCHAIN,
				RECREATE_RENDERPASS,
				RECREATE_FRAMEBUFFERS,
				RECREATE_CLEAR_VALUES,
				UPDATE_PROJECTION_MATRIX,

				MODIFICATION_COUNT
			};

			std::bitset<MODIFICATION_COUNT> modifications;

			if(extent != ext) {
				//Resolution has changed
				extent = ext;

				modifications.set(RECREATE_SWAPCHAIN);
				modifications.set(RECREATE_RENDERPASS);
				modifications.set(UPDATE_PROJECTION_MATRIX);
			}

			if(colorFormat != colorFmt) {
				//Format has changed
				colorFormat = colorFmt;

				modifications.set(RECREATE_SWAPCHAIN);
				modifications.set(RECREATE_RENDERPASS);
			}

			if(colorSpace != cs) {
				//Color space has changed
				colorSpace = cs;

				modifications.set(RECREATE_SWAPCHAIN);
			}

			if(colorTransfer != ct) {
				//Color transfer has changed
				colorTransfer = std::move(ct);

				modifications.set(RECREATE_RENDERPASS);
			}

			if(depthStencilFormat != depthStencilFmt) {
				//Depth/Stencil format has changed
				depthStencilFormat = depthStencilFmt;

				modifications.set(RECREATE_RENDERPASS);
				modifications.set(RECREATE_CLEAR_VALUES);
			}



			//Recreate stuff accordingly
			if(modifications.any()) {
				//Wait until rendering finishes
				waitCompletion();

				if(modifications.test(RECREATE_SWAPCHAIN)) {
					const auto oldExtent = extent;

					if(extent != vk::Extent2D(0, 0) && colorFormat != vk::Format::eUndefined) {
						swapchain = createSwapchain(vulkan, *surface, extent, colorFormat, colorSpace, *swapchain);
						swapchainImages = createSwapchainImages(vulkan, *swapchain, extent, colorFormat);
					} else {
						swapchain.reset();
						swapchainImages.clear();
					}
					
					modifications.set(RECREATE_FRAMEBUFFERS);

					//Extent might have changed
					if(oldExtent != extent) {
						modifications.set(RECREATE_RENDERPASS);
						modifications.set(UPDATE_PROJECTION_MATRIX);
					}
				}

				if(modifications.test(RECREATE_RENDERPASS)) {
					if(colorFormat != vk::Format::eUndefined) {
						renderPass = createRenderPass(vulkan, extent, colorFormat, colorTransfer, depthStencilFormat);
					} else {
						renderPass = Graphics::RenderPass();
					}
					
					modifications.set(RECREATE_FRAMEBUFFERS);
				}

				if(modifications.test(RECREATE_FRAMEBUFFERS)) {
					if(renderPass.get() && swapchainImages.size()) {
						framebuffers = createFramebuffers(vulkan, swapchainImages, renderPass);
					} else {
						framebuffers.clear();
					}
				}

				if(modifications.test(RECREATE_CLEAR_VALUES)) {
					clearValues = Graphics::RenderPass::getClearValues(depthStencilFormat);
				}

				if(modifications.test(UPDATE_PROJECTION_MATRIX)) {
					updateProjectionMatrixUniform(cam);
				}
				
			}
		}

		void setCamera(const Window::Camera& camera) {
			updateProjectionMatrixUniform(camera);
		}

		void draw(RendererBase& renderer) {
			//Wait until any previous rendering has finished
			waitCompletion();

			//Acquire an image from the swapchain
			size_t index = acquireImage();

			if(index < framebuffers.size()) {
				const auto frameBuffer = framebuffers[index].get();

				//Begin writing to the command buffer. //TODO maybe reset pool?
				constexpr vk::CommandBufferBeginInfo cmdBegin(
					vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 
					nullptr
				);
				commandBuffer.begin(cmdBegin);

				//Begin a render pass
				const vk::RenderPassBeginInfo rendBegin(
					renderPass.get(),													//Renderpass
					frameBuffer,														//Target framebuffer
					vk::Rect2D({0, 0}, extent),											//Extent
					clearValues.size(), clearValues.data()								//Attachment clear values
				);
				commandBuffer.beginRenderPass(rendBegin, vk::SubpassContents::eInline);

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

				//Evaluate if there are any layers and if so, draw them
				if(!renderer.getLayers().empty()) {
					//Flush the unform buffer
					uniformBuffer.flush(vulkan);

					//Bind the descriptor set
					commandBuffer.bindDescriptorSets(
						vk::PipelineBindPoint::eGraphics,					//Pipeline bind point
						pipelineLayout,										//Pipeline layout
						RendererBase::DESCRIPTOR_SET,						//First index
						uniformDescriptorSet,								//Descriptor sets
						{}													//Dynamic offsets
					);

					//Draw all the layers
					renderer.draw(commandBuffer);
				}

				//Finalize the renderpass if needed
				renderPass.finalize(vulkan, commandBuffer.get());

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
		void updateProjectionMatrixUniform(const Window::Camera& cam) {
			uniformBuffer.waitCompletion(vulkan);

			const auto size = Math::Vec2f(extent.width, extent.height);
			const auto mtx = cam.calculateMatrix(size);
			uniformBuffer.write(
				vulkan,
				RendererBase::DESCRIPTOR_BINDING_PROJECTION_MATRIX,
				&mtx,
				sizeof(mtx)
			);
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


		
		static GLFW::Window createWindow(	Math::Vec2i size, 
											const std::string& title,
											Window::Monitor monitor,
											WindowImpl& impl )
		{
			const GLFW::WindowCallbacks callbacks = {
				windowPositionCallback,
				windowSizeCallback,
				windowShouldCloseCallback,
				windowRefreshCallback,
				windowFocusCallback,
				windowIconifyCallback,
				windowMaximizeCallback,
				nullptr,
				windowScaleCallback,
				windowKeyCallback,
				windowCharCallback,
				windowMousePositionCallback,
				windowMouseEnterCallback,
				windowMouseButtonCallback,
				windowMouseScrollCallback
			};

			return GLFW::Window(
				size, 
				title.c_str(), 
				reinterpret_cast<const GLFW::Monitor&>(monitor), 
				callbacks, 
				&impl
			);
		}

		static vk::UniqueSurfaceKHR createSurface(	const Graphics::Vulkan& vulkan,
													const GLFW::Window& window )
		{
			auto surface = window.createSurface(vulkan.getInstance());

			if(!surface) {
				throw Exception("Unable to create a surface for the window");
			}

			using Deleter = vk::UniqueHandleTraits<vk::SurfaceKHR, vk::DispatchLoaderDynamic>::deleter;
			return vk::UniqueSurfaceKHR(
				surface,
				Deleter(vulkan.getInstance(), nullptr, vulkan.getDispatcher())
			);
		}

		static vk::UniqueCommandPool createCommandPool(const Graphics::Vulkan& vulkan) {
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

		static vk::UniqueDescriptorPool createDescriptorPool(const Graphics::Vulkan& vulkan){
			std::vector<vk::DescriptorPoolSize> poolSizes;
			poolSizes.insert(
				poolSizes.cend(), 
				RendererBase::getDescriptorPoolSizes().cbegin(),
				RendererBase::getDescriptorPoolSizes().cend() 
			);

			const vk::DescriptorPoolCreateInfo createInfo(
				{},														//Flags
				2,														//Descriptor set count
				poolSizes.size(), poolSizes.data()						//Pool sizes
			);

			return vulkan.createDescriptorPool(createInfo);
		}

		static vk::DescriptorSet createUniformDescriptorSet(const Graphics::Vulkan& vulkan,
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

		static std::vector<Graphics::Image> createSwapchainImages(	const Graphics::Vulkan& vulkan,
																	vk::SwapchainKHR swapchain,
																	vk::Extent2D extent,
																	vk::Format format ) 
		{
			const auto images = vulkan.getDevice().getSwapchainImagesKHR(swapchain, vulkan.getDispatcher());
			std::vector<Graphics::Image> result;
			result.reserve(images.size());

			std::transform(
				images.cbegin(), images.cend(),
				std::back_inserter(result),
				[&vulkan, extent, format] (vk::Image image) -> Graphics::Image {
					const Graphics::Image::Plane plane(
						Graphics::to3D(extent),
						format,
						vk::ComponentMapping(),
						image,
						vk::ImageView()
					);

					constexpr vk::ImageUsageFlags usage = 
						vk::ImageUsageFlagBits::eColorAttachment ;

					constexpr vk::ImageTiling tiling = vk::ImageTiling::eOptimal;

					constexpr vk::MemoryPropertyFlags memory = {};

					return Graphics::Image(
						vulkan,
						plane,
						usage,
						tiling,
						memory
					);
				}
			);
			assert(result.size() == images.size());

			return result;
		}

		static Graphics::RenderPass createRenderPass(	const Graphics::Vulkan& vulkan, 
														vk::Extent2D extent,
														vk::Format colorFormat,
														const Graphics::ColorTransferWrite& colorTransfer,
														DepthStencilFormat depthStencilFmt )
		{
			const Graphics::Image::Plane plane(Graphics::to3D(extent), colorFormat);

			return Graphics::RenderPass(
				vulkan,
				colorTransfer,
				plane,
				depthStencilFmt,
				vk::ImageLayout::ePresentSrcKHR
			);
		}

		static std::vector<vk::UniqueFramebuffer> createFramebuffers(	const Graphics::Vulkan& vulkan,
																		const std::vector<Graphics::Image>& swapchainImages,
																		const Graphics::RenderPass& renderPass )
		{
			std::vector<vk::UniqueFramebuffer> result;
			result.reserve(swapchainImages.size());

			std::transform(
				swapchainImages.cbegin(), swapchainImages.cend(),
				std::back_inserter(result),
				[&vulkan, &renderPass] (const Graphics::Image& target) -> vk::UniqueFramebuffer {
					return renderPass.createFramebuffer(vulkan, target);
				}
			);

			assert(swapchainImages.size() == result.size());
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
	};

	std::reference_wrapper<Window>				owner;

	std::string									title;
	Math::Vec2i 								size;
	Math::Vec2i 								position;
	float										opacity;
	bool										resizeable;
	bool										decorated;
	bool										visible;
	Window::Monitor								monitor;

	Window::Callbacks							callbacks;
	
	std::unique_ptr<Open>						opened;
	bool										hasChanged;


	static constexpr auto PRIORITY = Instance::OUTPUT_PRIORITY;
	static constexpr auto NO_POSTION = Math::Vec2i(std::numeric_limits<int32_t>::min());

	WindowImpl(	Window& owner,
				Instance& instance,
				Math::Vec2i size,
				const Window::Monitor& mon )
		: owner(owner)
		, title(instance.getApplicationInfo().getName())
		, size(size)
		, position(NO_POSTION)
		, opacity(1.0f)
		, resizeable(true)
		, decorated(true)
		, visible(true)
		, monitor(mon)
		, callbacks()
	{
	}

	~WindowImpl() = default;


	void moved(ZuazoBase& base) {
		owner = static_cast<Window&>(base);
	}

	void open(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		Window& window = static_cast<Window&>(base);
		assert(&owner.get() == &window);
		assert(!opened);

		//Create it in a unlocked environment
		if(lock) lock->unlock();
		auto newOpened = std::make_unique<Open>(
			window.getInstance(),
			size,
			title,
			monitor,
			*this,
			window.getCamera()
		);
		
		//Set everything as desired
		//newOpened->window.setName(windowName); //Already set when constructing
		//newOpened->window.setSize(size); //Already set when constructing
		if(position != NO_POSTION) newOpened->window.setPosition(position);
		newOpened->window.setOpacity(opacity);
		newOpened->window.setResizeable(resizeable);
		newOpened->window.setDecorated(decorated);
		newOpened->window.setVisibility(visible);
		if(lock) lock->lock();

		//Write changes after locking back
		opened = std::move(newOpened);
		window.setVideoModeCompatibility(getVideoModeCompatibility());

		hasChanged = true;

		assert(opened);
	}

	void asyncOpen(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		open(base, &lock);
		assert(lock.owns_lock());
	}

	void close(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		Window& window = static_cast<Window&>(base);
		assert(&owner.get() == &window);
		assert(opened);

		window.disablePeriodicUpdate();
		window.setViewportSize(Math::Vec2f());
		window.setRenderPass(vk::RenderPass());
		auto oldOpened = std::move(opened);

		if(lock) lock->unlock();
		oldOpened.reset();
		if(lock) lock->lock();

		assert(!opened);
	}

	void asyncClose(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		close(base, &lock);
		assert(lock.owns_lock());
	}

	void setVideoMode(VideoBase& base, const VideoMode& videoMode) {
		auto& window = static_cast<Window&>(base);
		recreate(window, videoMode, window.getDepthStencilFormat());
	}

	void setDepthStencilFormat(RendererBase& base, DepthStencilFormat depthStencil) {
		auto& window = static_cast<Window&>(base);
		recreate(window, window.getVideoMode(), depthStencil);
	}

	void setCamera(RendererBase& base, const RendererBase::Camera& camera) {
		Window& window = static_cast<Window&>(base);
		assert(&owner.get() == &window);

		if(opened) {
			opened->setCamera(camera);
			hasChanged = true;
		}
	}

	void update() {
		assert(opened);
		auto& window = owner.get();

		if(hasChanged || window.layersHaveChanged()) {
			opened->draw(window);

			hasChanged = false;
		}
	}

	std::vector<VideoMode> getVideoModeCompatibility() const {
		std::vector<VideoMode> result;

		if(opened) {
			//Select a monitor to depend on
			const auto& mon = (monitor != Window::Monitor()) ? monitor : Window::getPrimaryMonitor();

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

	void setTitle(std::string name) {
		if(title != name) {
			title = std::move(name);
			if(opened) opened->window.setTitle(title.c_str());
		}
	}

	const std::string& getTitle() const {
		return title;
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


	Math::Vec2f getScale() const {
		return opened ? opened->window.getScale() : Math::Vec2f(0.0f);
	}

	void setScaleCallback(Window::ScaleCallback cbk) {
		callbacks.scaleCbk = std::move(cbk);
	}

	const Window::ScaleCallback& getScaleCallback() const {
		return callbacks.scaleCbk;
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


	void setVisibility(bool visibility) {
		visible = visibility;
		if(opened) opened->window.setVisibility(visibility);
	}

	bool getVisibility() const {
		return visible;
	}
	

	void iconify() {
		if(opened) opened->window.iconify();
	}

	bool isIconified() const {
		return opened ? opened->window.isIconified() : false;
	}

	void setIconifyCallback(Window::IconifyCallback cbk) {
		callbacks.iconifyCbk = std::move(cbk);
	}

	const Window::IconifyCallback& getIconifyCallback() const {
		return callbacks.iconifyCbk;
	}


	void maximize() {
		if(opened) opened->window.maximize();
	}

	bool isMaximized() const {
		return opened ? opened->window.isMaximized() : false;
	}

	void setMaximizeCallback(Window::MaximizeCallback cbk) {
		callbacks.maximizeCbk = std::move(cbk);
	}

	const Window::MaximizeCallback& getMaximizeCallback() const {
		return callbacks.maximizeCbk;
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


	void restore() {
		if(opened) opened->window.restore();
	}


	void setMonitor(const Window::Monitor& mon, const Window::Monitor::Mode* mode) {
		monitor = mon;

		if(opened) {
			opened->window.setMonitor(
				reinterpret_cast<const GLFW::Monitor&>(monitor), 
				reinterpret_cast<const GLFW::VideoMode*>(mode)
			);

			size = opened->window.getSize();
			owner.get().setVideoModeCompatibility(getVideoModeCompatibility()); //This will call reconfigure if resizeing is needed
		}
	}
	
	Window::Monitor getMonitor() const {
		return monitor;
	}



	KeyEvent getKeyState(KeyboardKey key) const {
		return opened 
		? fromGLFW(opened->window.getKeyState(toGLFW(key))) 
		: KeyEvent::RELEASE;
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


	KeyEvent getMouseButtonState(MouseKey but) const {
		return opened 
		? fromGLFW(opened->window.getMouseButtonState(toGLFW(but)))
		: KeyEvent::RELEASE;
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
		const auto monitor = GLFW::Monitor::getPrimaryMonitor();
		return reinterpret_cast<const Window::Monitor&>(monitor);
	}

	static Utils::BufferView<const Window::Monitor> getMonitors() {
		const auto monitors = GLFW::Monitor::getMonitors();
		return Utils::BufferView<const Window::Monitor>(
			reinterpret_cast<const Window::Monitor*>(monitors.data()),
			monitors.size()
		);
	}

private:
	void recreate(	Window& window, 
					const VideoMode& videoMode, 
					DepthStencilFormat depthStencil )
	{
		assert(&owner.get() == &window);

		if(opened) {
			window.disablePeriodicUpdate();

			if(videoMode) {
				const auto frameDesc = videoMode.getFrameDescriptor();
				auto [extent, colorFormat, colorSpace, colorTransfer] = convertParameters(window.getInstance().getVulkan(), frameDesc);
				const auto framePeriod = getPeriod(videoMode.getFrameRateValue());

				//Update the parameters
				opened->recreate(
					extent, 
					colorFormat, 
					colorSpace, 
					std::move(colorTransfer), 
					depthStencil,
					window.getCamera()
				);

				window.enablePeriodicUpdate(PRIORITY, framePeriod);
			} else {
				//Unset the stuff
				opened->recreate(
					vk::Extent2D(0, 0), 
					vk::Format::eUndefined, 
					static_cast<vk::ColorSpaceKHR>(-1), 
					Graphics::ColorTransferWrite(), 
					DepthStencilFormat::NONE,
					window.getCamera()
				);
			}

			//Update the viewport size and the renderpass
			window.setViewportSize(opened ? Graphics::fromVulkan(opened->extent)  : Math::Vec2i());
			window.setRenderPass(opened ? opened->renderPass.get() : vk::RenderPass());

			hasChanged = true;
		}
	}

	void updateVideoMode() {
		auto& window = owner.get();
		window.setVideoModeCompatibility(getVideoModeCompatibility());
	}

	static std::tuple<vk::Extent2D, vk::Format, vk::ColorSpaceKHR, Graphics::ColorTransferWrite>
	convertParameters(	const Graphics::Vulkan& vulkan,
						const Graphics::Frame::Descriptor& frameDescriptor )
	{
		//Obtain the pixel format
		auto planes = frameDescriptor.getPlanes();
		assert(planes.size() == 1);

		auto& plane = planes.front();
		const auto[format, swizzle] = Graphics::optimizeFormat(std::make_tuple(plane.getFormat(), plane.getSwizzle()));
		plane.setFormat(format);
		plane.setSwizzle(swizzle);
		assert(plane.getSwizzle() == vk::ComponentMapping());

		//Obtain the color space
		const auto colorSpace = Graphics::toVulkan(
			frameDescriptor.getColorPrimaries(), 
			frameDescriptor.getColorTransferFunction()
		);

		//Create the color transfer characteristics
		Graphics::ColorTransferWrite colorTransfer(frameDescriptor);

		constexpr vk::FormatFeatureFlags DESIRED_FLAGS = 
			vk::FormatFeatureFlagBits::eColorAttachment;
		const auto& supportedFormats = vulkan.listSupportedFormatsOptimal(DESIRED_FLAGS);
		colorTransfer.optimize(planes, supportedFormats);

		return std::make_tuple(
			Graphics::to2D(plane.getExtent()), 
			plane.getFormat(), 
			colorSpace, 
			std::move(colorTransfer)
		);
	}


	static WindowImpl& getUserPointer(GLFW::WindowHandle win) {
		auto* usrPtr = static_cast<WindowImpl*>(GLFW::Instance::get().getUserPointer(win));
		assert(usrPtr);
		return *usrPtr;
	}

	static size_t getEmitterId(const WindowImpl& impl) {
		return reinterpret_cast<uintptr_t>(&impl);
	}

	static constexpr auto invokeIf = [] (auto&& f, auto&& ...params) {
		Utils::invokeIf(std::forward<decltype(f)>(f), std::forward<decltype(params)>(params)...);
	};

	static void windowPositionCallback(GLFW::WindowHandle win, int x, int y) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getPositionCallback()), std::ref(window), Math::Vec2i(x, y))
		);
	}

	static void windowSizeCallback(GLFW::WindowHandle win, int x, int y) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getSizeCallback()), std::ref(window), Math::Vec2i(x, y))
		);
	}

	static void windowShouldCloseCallback(GLFW::WindowHandle win) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getShouldCloseCallback()), std::ref(window))
		);
	}

	static void windowRefreshCallback(GLFW::WindowHandle win) {
		auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(&WindowImpl::updateVideoMode, std::ref(impl))
		);
	}

	static void windowFocusCallback(GLFW::WindowHandle win, int focus) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getFocusCallback()), std::ref(window), focus)
		);
	}

	static void windowIconifyCallback(GLFW::WindowHandle win, int iconify) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getIconifyCallback()), std::ref(window), iconify)
		);
	}

	static void windowMaximizeCallback(GLFW::WindowHandle win, int maximized) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getMaximizeCallback()), std::ref(window), maximized)
		);
	}

	static void windowScaleCallback(GLFW::WindowHandle win, float x, float y) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();
		
		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getScaleCallback()), std::ref(window), Math::Vec2f(x, y))
		);
	}

	static void windowKeyCallback(	GLFW::WindowHandle win, 
									GLFW::KeyboardKey key, 
									int scancode, 
									GLFW::KeyEvent event, 
									GLFW::KeyModifiers modifiers)
	{
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		Utils::ignore(scancode); //TODO Not implemented
		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getKeyboardCallback()), std::ref(window), fromGLFW(key), fromGLFW(event), fromGLFW(modifiers))
		);
	}

	static void windowCharCallback(GLFW::WindowHandle win, unsigned int character) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getCharacterCallback()), std::ref(window), character)
		);
	}

	static void windowMousePositionCallback(GLFW::WindowHandle win, double x, double y) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getMousePositionCallback()), std::ref(window), Math::Vec2d(x, y))
		);
	}

	static void windowMouseEnterCallback(GLFW::WindowHandle win, int entered) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getCursorEnterCallback()), std::ref(window), entered)
		);
	}

	static void windowMouseButtonCallback(	GLFW::WindowHandle win, 
											GLFW::MouseButton but,
											GLFW::KeyEvent event, 
											GLFW::KeyModifiers modifiers) 
	{
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();

		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getMouseButtonCallback()), std::ref(window), fromGLFW(but), fromGLFW(event), fromGLFW(modifiers))
		);
	}

	static void windowMouseScrollCallback(GLFW::WindowHandle win, double x, double y) {
		const auto& impl = getUserPointer(win);
		auto& window = static_cast<Window&>(impl.owner);
		auto& instance = window.getInstance();
		
		instance.addEvent(
			getEmitterId(impl),
			std::bind(invokeIf, std::cref(window.getMouseScrollCallback()), std::ref(window), Math::Vec2d(x, y))
		);
	}


};


/*
 * Window
 */

const Window::Monitor Window::NO_MONITOR = Window::Monitor();

Window::Window(	Instance& instance, 
				std::string name, 
				Math::Vec2i size,
				const Monitor& mon )
	: Utils::Pimpl<WindowImpl>({}, *this, instance, size, mon)
	, ZuazoBase(
		instance, std::move(name), 
		{},
		std::bind(&WindowImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&WindowImpl::open, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&WindowImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&WindowImpl::close, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&WindowImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&WindowImpl::update, std::ref(**this)) )
	, VideoBase(
		std::bind(&WindowImpl::setVideoMode, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
	, RendererBase(
		std::bind(&WindowImpl::setDepthStencilFormat, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&WindowImpl::setCamera, std::ref(**this), std::placeholders::_1, std::placeholders::_2)
	)
{
	setVideoModeCompatibility((*this)->getVideoModeCompatibility());
}

Window::Window(Window&& other) = default;

Window::~Window() = default;

Window& Window::operator=(Window&& other) = default;


void Window::setTitle(std::string name) {
	(*this)->setTitle(std::move(name));
}
const std::string& Window::getTitle() const {
	return (*this)->getTitle();
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


Math::Vec2f Window::getScale() const {
	return (*this)->getScale();
}

void Window::setScaleCallback(ScaleCallback cbk) {
	(*this)->setScaleCallback(std::move(cbk));
}

const Window::ScaleCallback& Window::getScaleCallback() const {
	return (*this)->getScaleCallback();
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


void Window::setVisibility(bool visibility) {
	(*this)->setVisibility(visibility);
}

bool Window::getVisibility() const {
	return (*this)->getVisibility();
}


void Window::iconify() {
	(*this)->iconify();
}

bool Window::isIconified() const {
	return (*this)->isIconified();
}

void Window::setIconifyCallback(IconifyCallback cbk) {
	(*this)->setIconifyCallback(std::move(cbk));
}

const Window::IconifyCallback& Window::getIconifyCallback() const {
	return (*this)->getIconifyCallback();
}


void Window::maximize() {
	(*this)->maximize();
}

bool Window::isMaximized() const {
	return (*this)->isMaximized();
}

void Window::setMaximizeCallback(MaximizeCallback cbk) {
	(*this)->setMaximizeCallback(std::move(cbk));
}

const Window::MaximizeCallback& Window::getMaximizeCallback() const {
	return (*this)->getMaximizeCallback();
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


void Window::restore() {
	(*this)->restore();
}


void Window::setMonitor(const Monitor& mon, const Monitor::Mode* mode) {
	(*this)->setMonitor(mon, mode);
}

Window::Monitor Window::getMonitor() const {
	return (*this)->getMonitor();
}



KeyEvent Window::getKeyState(KeyboardKey key) const {
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


KeyEvent Window::getMouseButtonState(MouseKey but) const {
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

Utils::BufferView<const Window::Monitor> Window::getMonitors() {
	return WindowImpl::getMonitors();
}

}