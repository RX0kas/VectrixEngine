#ifndef VECTRIXWORKSPACE_COMPUTESHADER_H
#define VECTRIXWORKSPACE_COMPUTESHADER_H
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

/**
 * @file ComputeShader.h
 * @brief Definition of ComputeShader, a GLSL compute shader run on the GPU
 * @ingroup physics
 */

namespace Vectrix {
	/**
	 * @brief A compute shader (`.vccompute`), with the storage buffers it reads and writes
	 *
	 * Load it with `AssetsManager::load<ComputeShader>(path)`. The shader sees:
	 * - up to k_maxBuffers storage buffers, `layout(std430, set = 0, binding = N) buffer ...` with N from 0 to
	 *   k_maxBuffers - 1, filled with setBuffer (or allocateBuffer for an output only) and read back with readBuffer;
	 * - up to k_maxPushConstantSize bytes of push constants, `layout(push_constant) uniform ...`, set with
	 *   setPushConstants (small parameters: a count, a time step...).
	 *
	 * @code
	 * // doubler.vccompute
	 * #version 450
	 * layout(local_size_x = 64) in;
	 * layout(std430, set = 0, binding = 0) buffer Values { float values[]; };
	 * layout(push_constant) uniform Params { uint count; } params;
	 * void main() {
	 *     uint i = gl_GlobalInvocationID.x;
	 *     if (i < params.count) values[i] *= 2.0;
	 * }
	 * @endcode
	 * @code
	 * auto [result, shader] = Vectrix::AssetsManager::load<Vectrix::ComputeShader>("shaders/doubler.vccompute");
	 * std::vector<float> values = {1, 2, 3};
	 * shader->setBuffer(0, values);
	 * const uint32_t count = static_cast<uint32_t>(values.size());
	 * shader->setPushConstants(count);
	 * shader->dispatch(Vectrix::ComputeShader::groupCount(count, 64));
	 * std::vector<float> doubled = shader->readBuffer<float>(0); // {2, 4, 6}
	 * @endcode
	 * @note dispatch waits for the GPU to finish: what readBuffer gives is the result. A shader and its buffers are
	 *       used by one thread at a time (any thread)
	 * @ingroup physics
	 */
	class ComputeShader {
	public:
		/// How many storage buffers a compute shader can use (bindings 0 to k_maxBuffers - 1 of set 0)
		static constexpr uint32_t k_maxBuffers = 8;
		/// The most push constant bytes setPushConstants takes (the minimum every Vulkan device offers)
		static constexpr size_t k_maxPushConstantSize = 128;

		virtual ~ComputeShader() = default;

		/**
		 * @brief Copy data into the storage buffer of a binding, creating or growing it as needed
		 * @param binding The buffer's binding in the shader, below k_maxBuffers
		 * @param data The bytes to copy
		 * @param size How many bytes, above 0. The buffer becomes this size
		 * @return false when the binding is out of range or the buffer can't be created (logged)
		 */
		virtual bool setBuffer(uint32_t binding, const void* data, size_t size) = 0;

		/// @brief setBuffer from an array of plain values (ints, floats, glm vectors, structs laid out as std430)
		template<typename T> requires std::is_trivially_copyable_v<T>
		bool setBuffer(const uint32_t binding, std::span<const T> data) {
			return setBuffer(binding, data.data(), data.size_bytes());
		}

		template<typename T> requires std::is_trivially_copyable_v<T>
		bool setBuffer(const uint32_t binding, const std::vector<T>& data) {
			return setBuffer(binding, std::span<const T>(data));
		}

		/**
		 * @brief Give a binding a buffer of the given size without filling it, for a result the shader writes
		 * @param binding The buffer's binding in the shader, below k_maxBuffers
		 * @param size How many bytes, above 0. A buffer that already has this size keeps its content
		 * @return false when the binding is out of range or the buffer can't be created (logged)
		 */
		virtual bool allocateBuffer(uint32_t binding, size_t size) = 0;

		/// @brief The size in bytes of a binding's buffer, 0 when it has none
		[[nodiscard]] virtual size_t getBufferSize(uint32_t binding) const = 0;

		/**
		 * @brief Set the push constants the next dispatches get
		 * @param data The bytes, as the shader's `push_constant` block lays them out
		 * @param size At most k_maxPushConstantSize
		 * @return false when it's too big (logged)
		 */
		virtual bool setPushConstants(const void* data, size_t size) = 0;

		template<typename T> requires std::is_trivially_copyable_v<T>
		bool setPushConstants(const T& value) {
			static_assert(sizeof(T) <= k_maxPushConstantSize, "Push constants are limited to 128 bytes");
			return setPushConstants(&value, sizeof(T));
		}

		/**
		 * @brief Run the shader on groupsX * groupsY * groupsZ work groups, and wait for it to finish
		 * @return false when it couldn't run (a buffer the shader reads is missing is not detected: give it every
		 *         binding it uses)
		 * @see groupCount
		 */
		virtual bool dispatch(uint32_t groupsX, uint32_t groupsY = 1, uint32_t groupsZ = 1) = 0;

		/**
		 * @brief Copy a binding's buffer (what the last dispatch left in it) into memory
		 * @param binding The buffer's binding
		 * @param out Where to copy
		 * @param size How many bytes
		 * @param offset From which byte of the buffer
		 * @return false when the binding has no buffer or the range goes past its end (logged)
		 */
		virtual bool readBuffer(uint32_t binding, void* out, size_t size, size_t offset = 0) const = 0;

		/// @brief A binding's whole buffer as an array of T, empty when there is none
		template<typename T> requires std::is_trivially_copyable_v<T>
		std::vector<T> readBuffer(const uint32_t binding) const {
			std::vector<T> values(getBufferSize(binding) / sizeof(T));
			if (!values.empty() && !readBuffer(binding, values.data(), values.size() * sizeof(T)))
				values.clear();
			return values;
		}

		/// @brief How many work groups of groupSize invocations cover count items
		static constexpr uint32_t groupCount(const uint32_t count, const uint32_t groupSize) {
			return (count + groupSize - 1) / groupSize;
		}
	private:
		friend class ComputeShaderManager;
		friend class AssetsManager;
		/// @return The shader, or nullptr when it doesn't compile (the error is logged)
		static std::shared_ptr<ComputeShader> create(const std::string& name, const std::string& path);
		static std::shared_ptr<ComputeShader> createFromSource(const std::string& name, const std::string& source);
	};
} // Vectrix

#endif //VECTRIXWORKSPACE_COMPUTESHADER_H
