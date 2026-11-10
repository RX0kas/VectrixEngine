#ifndef VECTRIXWORKSPACE_ASSETLOAD_H
#define VECTRIXWORKSPACE_ASSETLOAD_H

#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Vectrix/Threading/JobSystem.h"
#include "Vectrix/Utils/Result.h"

/**
 * @file AssetLoad.h
 * @brief Definition of AssetLoad, the handle of an asset being loaded in the background
 * @ingroup assets
 */

namespace Vectrix {
	/// @cond INTERNAL
	namespace detail {
		// One load, shared by the handles asking for the same file. Fields up to `finish` are set before the job
		// starts; `finish` by the job; the rest on the main thread only
		struct AssetLoadState {
			int type = 0; ///< The AssetType
			std::string id;
			std::string cacheKey;
			bool inAssetsFolder = false;
			std::uint64_t assetsFolderGeneration = 0; ///< Not registered if the assets folder changed meanwhile
			JobHandle job;
			/// Set by the job: what's left to do on the main thread (e.g. building a mesh's GPU buffers)
			std::function<std::pair<VectrixResult, std::shared_ptr<void>>()> finish;

			bool done = false;
			VectrixResult result = SUCCESS;
			std::shared_ptr<void> asset;
			std::vector<std::function<void()>> callbacks; ///< Run once done
		};

		// AssetsManager.cpp: block the main thread until the load is done, helping the job system meanwhile
		void waitForAssetLoad(const std::shared_ptr<AssetLoadState>& state);
	}
	/// @endcond

	/**
	 * @brief A load started by AssetsManager::loadAsync: tells when the asset is ready, and gives it
	 *
	 * The file is read and decoded on a worker thread (for a texture or a shader, its GPU objects are built there
	 * too); the asset is then registered on the main thread, at the start of a frame. Until then the handle isn't
	 * done, and get() gives nothing: show a stand-in meanwhile (e.g. TextureManager::getNotFoundTexture, or nothing).
	 *
	 * Copyable; every handle on the same file shares the same load. Main thread only, like the assets managers.
	 * @code
	 * Vectrix::AssetLoad<Vectrix::Texture> m_preview = Vectrix::AssetsManager::loadAsync<Vectrix::Texture>("textures/big.png");
	 * // every frame
	 * const auto texture = m_preview.isReady() ? m_preview.get() : Vectrix::TextureManager::getNotFoundTexture();
	 * @endcode
	 * @tparam T Texture, Shader or Mesh
	 * @see AssetsManager::loadAsync
	 * @ingroup assets
	 */
	template<typename T>
	class AssetLoad {
	public:
		/// A handle on nothing: done, with NOT_FOUND
		AssetLoad() = default;

		/// @brief Whether the load is over, successfully or not
		[[nodiscard]] bool isDone() const { return !m_state || m_state->done; }
		/// @brief Whether the asset is loaded and registered
		[[nodiscard]] bool isReady() const { return m_state && m_state->done && m_state->result == SUCCESS; }
		/// @brief How the load went: meaningful once isDone()
		[[nodiscard]] VectrixResult getResult() const { return m_state ? m_state->result : NOT_FOUND; }
		/// @brief The asset, null until isReady()
		[[nodiscard]] std::shared_ptr<T> get() const { return isReady() ? std::static_pointer_cast<T>(m_state->asset) : nullptr; }
		/// @brief The asset's id (see AssetsManager::getProjectAssetId), empty for an empty handle
		[[nodiscard]] const std::string& getId() const {
			static const std::string empty;
			return m_state ? m_state->id : empty;
		}

		/**
		 * @brief Call a function on the main thread once the load is over (right away if it already is)
		 * @param callback Called with this handle. It must not use what may be gone by then (see Application::runOnMainThread)
		 */
		template<std::invocable<const AssetLoad&> F>
		void then(F&& callback) const {
			if (isDone()) {
				std::invoke(callback, *this);
				return;
			}
			m_state->callbacks.emplace_back([handle = *this, callback = std::forward<F>(callback)]() mutable { std::invoke(callback, handle); });
		}

		/**
		 * @brief Block until the load is over, the calling thread helping the workers meanwhile
		 * @return The result and the asset, as AssetsManager::load gives them
		 * @note Main thread only
		 */
		std::pair<VectrixResult, std::shared_ptr<T>> wait() const {
			if (m_state && !m_state->done)
				detail::waitForAssetLoad(m_state);
			return {getResult(), get()};
		}
	private:
		friend class AssetsManager;
		explicit AssetLoad(std::shared_ptr<detail::AssetLoadState> state) : m_state(std::move(state)) {}
		std::shared_ptr<detail::AssetLoadState> m_state;
	};
}

#endif //VECTRIXWORKSPACE_ASSETLOAD_H
