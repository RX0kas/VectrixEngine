#pragma once

#include <string>

#include "ShaderUniformLayout.h"
#include "UniformTrait.h"
#include "Vectrix/Rendering/Buffer.h"
#include "Vectrix/Rendering/Camera/PerspectiveCamera.h"
#include "Vectrix/Rendering/Textures/Texture.h"

#define VC_VERIFY_UNIFORM_NAME(name) if (name=="cameraTransform" || name=="modelMat") VC_ERROR("The uniform name \"{}\" is reserved",name)


/**
 * @file Shader.h
 * @brief Definition of the shader class
 * @ingroup shaders
 */


namespace Vectrix {
	/**
 	* @brief This class represent a shader
 	* @details This class represent a shader that can be created with Vectrix::ShaderManager::createShader()
 	*/
	class Shader {
		public:
		virtual ~Shader() = default;

		/**
		 * @brief Tell the GPU that we need to use this shader \n
		 * This function tell the GPU that this shader will be used for all subsequent draw commands until another one is bound
		 * @warning Only one shader can be bound at a time, if you call bind on another shader it will stop using this one
		 * @warning The shader is automatically unbound at each new frame
		 */
		virtual void bind() const = 0;

		/**
		 * @brief Define the value of a boolean uniform in the shader
		 *
		 * This function permit to modify the value of a boolean uniform variable in this shader
		 *
		 * @param name Name of the value that is sent (ex: "u_LightEnabled")
		 * @param value Boolean value that is sent (true/false)
		 *
		 * @pre The shader must be bind before
		 * @pre The uniform must exist in the ShaderUniformLayout
		 *
		 * @post The value is set until another one is given, even for upcoming frames
		 *
		 * @warning Don't call this function before the initialization of the window
		 * @warning The name of the uniform is case-sensitive
		 *
		 * @see setUniform1i() For an integer
		 * @see setUniform1u() For an unsigned integer
		 * @see setUniform1f() For a floating-point value
		 * @see setUniform2f() For a 2D vector
		 * @see setUniform3f() For a 3D vector
		 * @see setUniform4f() For a 4D vector
		 * @see setUniformMat4f() For a 4x4 Matrices
		 * @see setUniform() To automatically detect the type of data
		 *
		 * @throws std::runtime_error if the uniform is not found or if it's not a boolean
		 */
		virtual void setUniformBool(const std::string& name, bool value) const = 0;

		/**
		 * @brief Define the value of an integer uniform in the shader
		 *
		 * This function permit to modify the value of an integer uniform variable in this shader
		 *
		 * @param name Name of the value that is sent (ex: "u_LightEnabled")
		 * @param value Integer value that is sent (-1/1)
		 *
		 * @pre The shader must be bind before
		 * @pre The uniform must exist in the ShaderUniformLayout
		 *
		 * @post The value is set until another one is given, even for upcoming frames
		 *
		 * @warning Don't call this function before the initialization of the window
		 * @warning The name of the uniform is case-sensitive
		 *
		 * @see setUniformBool() For a boolean
		 * @see setUniform1u() For an unsigned integer
		 * @see setUniform1f() For a floating-point value
		 * @see setUniform2f() For a 2D vector
		 * @see setUniform3f() For a 3D vector
		 * @see setUniform4f() For a 4D vector
		 * @see setUniformMat4f() For a 4x4 Matrices
		 * @see setUniform() To automatically detect the type of data
		 *
		 * @throws std::runtime_error if the uniform is not found or if it's not an integer
		 */
		virtual void setUniform1i(const std::string& name,int value) const = 0;
		/**
		 * @brief Define the value of an unsigned integer uniform in the shader
		 *
		 * This function permit to modify the value of an unsigned integer uniform variable in this shader
		 *
		 * @param name Name of the value that is sent (ex: "u_LightEnabled")
		 * @param value unsigned integer value that is sent (0/2)
		 *
		 * @pre The shader must be bind before
		 * @pre The uniform must exist in the ShaderUniformLayout
		 *
		 * @post The value is set until another one is given, even for upcoming frames
		 *
		 * @warning Don't call this function before the initialization of the window
		 * @warning The name of the uniform is case-sensitive
		 *
		 * @see setUniformBool() For a boolean
		 * @see setUniform1i() For an integer
		 * @see setUniform1f() For a floating-point value
		 * @see setUniform2f() For a 2D vector
		 * @see setUniform3f() For a 3D vector
		 * @see setUniform4f() For a 4D vector
		 * @see setUniformMat4f() For a 4x4 Matrices
		 * @see setUniform() To automatically detect the type of data
		 *
		 * @throws std::runtime_error if the uniform is not found or if it's not an unsigned integer
		 */
		virtual void setUniform1u(const std::string& name,unsigned int value) const = 0;
		/**
		 * @brief Define the value of a floating-point uniform in the shader
		 *
		 * This function permit to modify the value of a floating-point value uniform variable in this shader
		 *
		 * @param name Name of the value that is sent (ex: "u_LightEnabled")
		 * @param value floating-point value that is sent (ex: 0.2)
		 *
		 * @pre The shader must be bind before
		 * @pre The uniform must exist in the ShaderUniformLayout
		 *
		 * @post The value is set until another one is given, even for upcoming frames
		 *
		 * @warning Don't call this function before the initialization of the window
		 * @warning The name of the uniform is case-sensitive
		 *
		 * @see setUniformBool() For a boolean
		 * @see setUniform1i() For an integer
		 * @see setUniform1u() For an unsigned integer
		 * @see setUniform2f() For a 2D vector
		 * @see setUniform3f() For a 3D vector
		 * @see setUniform4f() For a 4D vector
		 * @see setUniformMat4f() For a 4x4 Matrices
		 * @see setUniform() To automatically detect the type of data
		 *
		 * @throws std::runtime_error if the uniform is not found or if it's not a floating-point value
		 */
		virtual void setUniform1f(const std::string& name,float value) const = 0;
		/**
		 * @brief Define the value of a 2D vector uniform in the shader
		 *
		 * This function permit to modify the value of a 2D vector uniform variable in this shader
		 *
		 * @param name Name of the value that is sent (ex: "u_LightEnabled")
		 * @param value 2D vector value that is sent ({0.5,0.0})
		 *
		 * @pre The shader must be bind before
		 * @pre The uniform must exist in the ShaderUniformLayout
		 *
		 * @post The value is set until another one is given, even for upcoming frames
		 *
		 * @warning Don't call this function before the initialization of the window
		 * @warning The name of the uniform is case-sensitive
		 *
		 * @see setUniformBool() For a boolean
		 * @see setUniform1i() For an integer
		 * @see setUniform1u() For an unsigned integer
		 * @see setUniform1f() For a floating-point value
		 * @see setUniform3f() For a 3D vector
		 * @see setUniform4f() For a 4D vector
		 * @see setUniformMat4f() For a 4x4 Matrices
		 * @see setUniform() To automatically detect the type of data
		 *
		 * @throws std::runtime_error if the uniform is not found or if it's not an 2D vector value
		 */
		virtual void setUniform2f(const std::string& name,glm::vec2 value) const = 0;
		/**
		 * @brief Define the value of a 3D vector uniform in the shader
		 *
		 * This function permit to modify the value of a 3D vector uniform variable in this shader
		 *
		 * @param name Name of the value that is sent (ex: "u_LightEnabled")
		 * @param value 3D vector value that is sent ({0.5,0.0,2.6})
		 *
		 * @pre The shader must be bind before
		 * @pre The uniform must exist in the ShaderUniformLayout
		 *
		 * @post The value is set until another one is given, even for upcoming frames
		 *
		 * @warning Don't call this function before the initialization of the window
		 * @warning The name of the uniform is case-sensitive
		 *
		 * @see setUniformBool() For a boolean
		 * @see setUniform1i() For an integer
		 * @see setUniform1u() For an unsigned integer
		 * @see setUniform1f() For a floating-point value
		 * @see setUniform2f() For a 2D vector
		 * @see setUniform4f() For a 4D vector
		 * @see setUniformMat4f() For a 4x4 Matrices
		 * @see setUniform() To automatically detect the type of data
		 *
		 * @throws std::runtime_error if the uniform is not found or if it's not an 3D vector value
		 */
		virtual void setUniform3f(const std::string& name,glm::vec3 value) const = 0;
		/**
		 * @brief Define the value of a 4D vector uniform in the shader
		 *
		 * This function permit to modify the value of a 4D vector uniform variable in this shader
		 *
		 * @param name Name of the value that is sent (ex: "u_LightEnabled")
		 * @param value 4D vector value that is sent ({0.5,0.0,2.6,1.0})
		 *
		 * @pre The shader must be bind before
		 * @pre The uniform must exist in the ShaderUniformLayout
		 *
		 * @post The value is set until another one is given, even for upcoming frames
		 *
		 * @warning Don't call this function before the initialization of the window
		 * @warning The name of the uniform is case-sensitive
		 *
		 * @see setUniformBool() For a boolean
		 * @see setUniform1i() For an integer
		 * @see setUniform1u() For an unsigned integer
		 * @see setUniform1f() For a floating-point value
		 * @see setUniform2f() For a 2D vector
		 * @see setUniform3f() For a 3D vector
		 * @see setUniformMat4f() For a 4x4 Matrices
		 * @see setUniform() To automatically detect the type of data
		 *
		 * @throws std::runtime_error if the uniform is not found or if it's not an 4D vector value
		 */
		virtual void setUniform4f(const std::string& name,glm::vec4 value) const = 0;
		/**
		 * @brief Define the value of a 4x4 matrix uniform in the shader
		 *
		 * This function permit to modify the value of a 4x4 matrix uniform variable in this shader
		 *
		 * @param name Name of the value that is sent (ex: "u_LightEnabled")
		 * @param value 4x4 matrix value that is sent
		 *
		 * @pre The shader must be bind before
		 * @pre The uniform must exist in the ShaderUniformLayout
		 *
		 * @post The value is set until another one is given, even for upcoming frames
		 *
		 * @warning Don't call this function before the initialization of the window
		 * @warning The name of the uniform is case-sensitive
		 *
		 * @see setUniformBool() For a boolean
		 * @see setUniform1i() For an integer
		 * @see setUniform1u() For an unsigned integer
		 * @see setUniform1f() For a floating-point value
		 * @see setUniform2f() For a 2D vector
		 * @see setUniform3f() For a 3D vector
		 * @see setUniform4f() For a 4D vector
		 * @see setUniform() To automatically detect the type of data
		 *
		 * @throws std::runtime_error if the uniform is not found or if it's not a 4x4 matrix value
		 */
		virtual void setUniformMat4f(const std::string& name,glm::mat4 value) const = 0;

		/**
		 * @brief Define the value of the current texture in the shader
		 *
		 * This function permit to modify the value of the current texture in this shader
		 *
		 * @param value texture that is sent
		 *
		 * @pre The shader must be bind before
		 *
		 * @post The value is set until another one is given, even for upcoming frames
		 *
		 * @warning Don't call this function before the initialization of the window
		 * @warning The name of the uniform is case-sensitive
		 *
		 * @warning <strong>For now only one texture per shader is supported</strong>
		 */
		virtual void setTexture(Ref<Texture> value) = 0;

		/**
		 * @brief Define the value of a uniform in the shader, without needing to think about the type
		 *
		 * This function permit to modify the value of a uniform variable in this shader
		 * @tparam T Any type that is supported @see ShaderUniformType
		 * @param name Name of the value that is sent (ex: "u_LightEnabled")
		 * @param value The data of the value that is sent
		 *
		 * @pre The shader must be bind before
		 * @pre The uniform must exist in the ShaderUniformLayout
		 *
		 * @post The value is set until another one is given, even for upcoming frames
		 *
		 * @warning Don't call this function before the initialization of the window
		 * @warning The name of the uniform is case-sensitive
		 *
		 * @see setUniformBool() For a boolean
		 * @see setUniform1i() For an integer
		 * @see setUniform1u() For an unsigned integer
		 * @see setUniform1f() For a floating-point value
		 * @see setUniform2f() For a 2D vector
		 * @see setUniform3f() For a 3D vector
		 * @see setUniform4f() For a 4D vector
		 * @see setUniformMat4f() For a 4x4 Matrices
		 *
		 * @throws std::runtime_error if the uniform is not found
		 */
		template<typename T>
		void setUniform(const std::string& name, const T& value) const {
			static_assert(UniformTraits<T>::valid, "Unsupported uniform type");
			setUniformImplementation(name,UniformTraits<T>::type,&value,sizeof(T));
		}

		/**
		 * @brief Return if the shader is set up to receives the camera
		 */
		[[nodiscard]] virtual bool isAffectedByCamera() const = 0;
	protected:
		/// @cond INTERNAL
		virtual void setUniformImplementation(const std::string& name,ShaderUniformType type,const void* data,size_t size) const = 0;
		static void finalize(ShaderUniformLayout* s) {
			s->finalize();
		}
		///  @endcond
	private:
		virtual void sendCameraUniform(const glm::mat4& camera) const = 0;
		virtual void setModelMatrix(const glm::mat4& model) const = 0;
		friend class ShaderManager;
		friend class Renderer;
		static Shader* create(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath,ShaderUniformLayout layout,const BufferLayout& buffer_layout,bool affectedByCamera);
	};
}
