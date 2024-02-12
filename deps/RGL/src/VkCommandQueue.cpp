#if RGL_VK_AVAILABLE
#include "VkCommandQueue.hpp"
#include "VkCommandBuffer.hpp"
#include "VkDevice.hpp"
#include "VkSynchronization.hpp"
#include "VkSwapchain.hpp"

namespace RGL {
    CommandQueueVk::CommandQueueVk(decltype(owningDevice) device) : owningDevice(device)
    {
        vkGetDeviceQueue(device->device, device->indices.graphicsFamily.value(), 0, &queue);    // 0 because we only have 1 queue
        VK_VALID(queue);
    }
    void CommandQueueVk::Submit(CommandBufferVk* cb, const CommitConfig& config, VkFence internalFence)
	{
        const uint32_t nSwapchains = cb->swapchainsToSignal.size();
        stackarray(waitSemaphores, VkSemaphore, nSwapchains);
        stackarray(signalSemaphores, VkSemaphore, nSwapchains);
        {
            uint32_t i = 0;
            for (const auto swapchain : cb->swapchainsToSignal) {
                waitSemaphores[i] = swapchain->imageAvailableSemaphore;
                signalSemaphores[i] = swapchain->renderCompleteSemaphore;
                i++;
            }
        }
       


        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submitInfo{
           .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
           .waitSemaphoreCount = nSwapchains,
           .pWaitSemaphores = waitSemaphores,
           .pWaitDstStageMask = waitStages,
           .commandBufferCount = 1,
           .pCommandBuffers = &(cb->commandBuffer),
           .signalSemaphoreCount = nSwapchains,
           .pSignalSemaphores = signalSemaphores
        };
        std::shared_ptr<FenceVk> fence;
        if (config.signalFence) {
            fence = std::static_pointer_cast<FenceVk>(config.signalFence);
        }
        VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence ? fence->fence : VK_NULL_HANDLE));
        VK_CHECK(vkQueueSubmit(queue,0, nullptr, internalFence));
	}
    RGLCommandBufferPtr CommandQueueVk::CreateCommandBuffer()
	{
		return std::make_shared<CommandBufferVk>(shared_from_this());
	}
    void CommandQueueVk::WaitUntilCompleted()
    {
        vkQueueWaitIdle(queue);
    }
    QueueData CommandQueueVk::GetQueueData()
    {
        return {
            .vkData{

            }
        };
    }
}

#endif
