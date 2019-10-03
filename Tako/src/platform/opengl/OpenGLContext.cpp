#include "GraphicsContext.hpp"

#include "Math.hpp"
#include "Bitmap.hpp"
#ifdef _WIN32
#include <Windows.h>
#include <glbinding/gl20/gl.h>
#include <glbinding/Binding.h>
using namespace gl20;

#pragma comment(lib, "opengl32.lib")
#endif
#include <GLES2/gl2.h>

namespace tako
{
	#ifdef _WIN32
	glbinding::ProcAddress GetGLProcAddress(const char* name)
	{
		auto procAddress = reinterpret_cast<glbinding::ProcAddress>(wglGetProcAddress(name));
		if (procAddress == nullptr)
		{
			static auto module = LoadLibrary("OPENGL32.DLL");
			procAddress = reinterpret_cast<glbinding::ProcAddress>(GetProcAddress(module, name));
		}

		return procAddress;
	}
	#endif

	static const Vector2 vertices[] =
	{
		{ 0, 0},
		{ 1, 0},
		{ 1, 1},
		{ 0, 0},
		{ 1, 1},
		{ 0, 1}
	};

	struct ImageVertex
	{
		Vector2 position;
		Vector2 texcoord;
	};

	static const ImageVertex imageVertices[] =
	{
		{ {0, 0}, {0, 0}},
		{ {1, 0}, {1, 0}},
		{ {1, 1}, {1, 1}},
		{ {0, 0}, {0, 0}},
		{ {1, 1}, {1, 1}},
		{ {0, 1}, {0, 1}}
	};

	constexpr const char* quadVertexShader =
		#include "quad.vert.glsl"
	;

	constexpr const char* quadFragmentShader =
		#include "quad.frag.glsl"    
	;

	constexpr const char* imageVertexShader =
		#include "image.vert.glsl"
	;

	constexpr const char* imageFragmentShader =
		#include "image.frag.glsl"    
	;



	class GraphicsContext::ContextImpl
	{
	public:
		ContextImpl(WindowHandle hwnd, int width, int height)
		{
			m_handle = hwnd;
			/*
			m_hdc = GetDC(hwnd);
			PIXELFORMATDESCRIPTOR pfd;

			memset(&pfd, 0, sizeof(pfd));
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;

			int pf = ChoosePixelFormat(m_hdc, &pfd);
			SetPixelFormat(m_hdc, pf, &pfd);
			DescribePixelFormat(m_hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
			ReleaseDC(hwnd, m_hdc);


			m_hrc = wglCreateContext(m_hdc);
			wglMakeCurrent(m_hdc, m_hrc);


			glbinding::Binding::initialize(GetGLProcAddress);
			*/
			glClearColor(0, 0.5f, 0, 1);


			glDisable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			//glEnable(GL_ALPHA_TEST);
			//glAlphaFunc(GL_GREATER, 0.1);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			

			SetupQuadPipeline();
			SetupImagePipeline();
			Resize(width, height);

            Bitmap tree = Bitmap::FromFile("tree.png");
            bitmap = UploadBitmap(tree);
		}

		void Resize(int w, int h)
		{
			glViewport(0, 0, w, h);

			auto err = glGetError();
			if (err != GL_NO_ERROR)
			{
				LOG("error preresize");
			}

			Matrix4 ortho = Matrix4::transpose(Matrix4::ortho(0, w, h, 0, 0, 100));
			glUseProgram(m_quadProgram);
			glUniformMatrix4fv(m_quadProjectionUniform, 1, GL_FALSE, &ortho[0]);
			glUseProgram(m_imageProgram);
			glUniformMatrix4fv(m_imageProjectionUniform, 1, GL_FALSE, &ortho[0]);
			err = glGetError();
			if (err != GL_NO_ERROR)
			{
				LOG("error resize");
			}
		}

		void DrawSquare(float x, float y, float w, float h, Color c)
		{
			glUseProgram(m_quadProgram);

			Matrix4 mat = Matrix4::identity;
			mat.translate(x, y, 0);
			mat.scale(w, h, 1);
			glUniformMatrix4fv(m_quadModelUniform, 1, GL_FALSE, &mat[0]);

			float col[4] = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
			glUniform4fv(m_quadColorUniform, 1, col);


			glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		void DrawImage(float x, float y, float w, float h, GLuint texture)
		{
			glUseProgram(m_imageProgram);

			Matrix4 mat = Matrix4::identity;
			mat.translate(x, y, 0);
			mat.scale(w, h, 1);
			glUniformMatrix4fv(m_imageModelUniform, 1, GL_FALSE, &mat[0]);

			//glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			//glUniform1i(m_imageTextureUniform, 0);

			glBindBuffer(GL_ARRAY_BUFFER, m_imageVBO);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImageVertex), NULL);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImageVertex), (void*)offsetof(ImageVertex, texcoord));
			glEnableVertexAttribArray(1);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		static GLuint UploadBitmap(const Bitmap& bitmap)
		{
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.Width(), bitmap.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetData());

			return texture;
		}


		static GLuint CompileShader(const char* shaderSource, GLenum type)
		{
			const GLchar* shaderPointer = shaderSource;
			GLuint sh = glCreateShader(type);
			glShaderSource(sh, 1, &shaderPointer, NULL);
			glCompileShader(sh);

			GLint isCompiled = 0;
			glGetShaderiv(sh, GL_COMPILE_STATUS, &isCompiled);
			if(isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> errorLog(maxLength);
				glGetShaderInfoLog(sh, maxLength, &maxLength, &errorLog[0]);
				LOG_ERR("{}", std::string(errorLog.begin(),errorLog.end()));

				// Provide the infolog in whatever manor you deem best.
				// Exit with failure.
				glDeleteShader(sh); // Don't leak the shader.
				return 0;
			}

			auto err = glGetError();
			if (err != GL_NO_ERROR)
			{
				LOG("error compiling shader!");
			}

			return sh;
		}

		void SetupQuadPipeline()
		{
			m_quadProgram = glCreateProgram();
			glAttachShader(m_quadProgram, CompileShader(quadVertexShader, GL_VERTEX_SHADER));
			glAttachShader(m_quadProgram, CompileShader(quadFragmentShader, GL_FRAGMENT_SHADER));
			glLinkProgram(m_quadProgram);
			auto err = glGetError();
			if (err != GL_NO_ERROR)
			{
				LOG("error shader");
			}

			m_quadVBO = 0;
			glGenBuffers(1, &m_quadVBO);
			glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			m_quadProjectionUniform = glGetUniformLocation(m_quadProgram, "projection");
			m_quadModelUniform = glGetUniformLocation(m_quadProgram, "model");
			m_quadColorUniform = glGetUniformLocation(m_quadProgram, "color");
		}

		void SetupImagePipeline()
		{
			m_imageProgram = glCreateProgram();
			glAttachShader(m_imageProgram, CompileShader(imageVertexShader, GL_VERTEX_SHADER));
			glAttachShader(m_imageProgram, CompileShader(imageFragmentShader, GL_FRAGMENT_SHADER));
			glLinkProgram(m_imageProgram);

			m_imageVBO = 0;
			glGenBuffers(1, &m_imageVBO);
			glBindBuffer(GL_ARRAY_BUFFER, m_imageVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(imageVertices), imageVertices, GL_STATIC_DRAW);

			m_imageProjectionUniform = glGetUniformLocation(m_imageProgram, "projection");
			m_imageModelUniform = glGetUniformLocation(m_imageProgram, "model");
			m_imageTextureUniform = glGetUniformLocation(m_imageProgram, "texture");

			auto err = glGetError();
			if (err != GL_NO_ERROR)
			{
				LOG("error pip!");
			}
		}

		void Draw()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			DrawSquare(100, 100, 100, 100, Color("#FFFF00AA"));
			DrawSquare(0, 0, 100, 100, Color("#00FFAA"));
			DrawSquare(50, 50, 100, 100, Color("#00FF5555"));
            DrawImage(175, 175, 67 * 2, 80 * 2, bitmap);
			//DrawSquare(200, 200, 100, 100, Color("#00FFAA"));


			auto err = glGetError();
			if (err != GL_NO_ERROR)
			{
				LOG("draw error!");
			}


			glFlush();
			//SwapBuffers(m_hdc);
		}

		void HandleEvent(Event& evt)
		{
			switch (evt.GetType())
			{
			case tako::EventType::WindowResize:
			{
				tako::WindowResize& res = static_cast<tako::WindowResize&>(evt);
				LOG("Window Resize: {} {} {}", res.GetName(), res.width, res.height);
				Resize(res.width, res.height);
				Draw();
			} break;
			}
		}
	private:
		//HDC m_hdc;
		//HGLRC m_hrc;
		GLuint bitmap;
		WindowHandle m_handle;

		GLuint m_quadProgram;
		GLuint m_quadVBO;
		GLuint m_quadProjectionUniform;
		GLuint m_quadModelUniform;
		GLuint m_quadColorUniform;

		GLuint m_imageProgram;
		GLuint m_imageVBO;
		GLuint m_imageProjectionUniform;
		GLuint m_imageModelUniform;
		GLuint m_imageTextureUniform;
	};

	GraphicsContext::GraphicsContext(WindowHandle handle, int width, int height) : m_impl(new ContextImpl(handle, width, height))
	{
	}

	GraphicsContext::~GraphicsContext() = default;

	void GraphicsContext::Present()
	{
		m_impl->Draw();
	}

	void GraphicsContext::Resize(int width, int height)
	{
		m_impl->Resize(width, height);
	}

	void GraphicsContext::HandleEvent(Event& evt)
	{
		m_impl->HandleEvent(evt);
	}
}