/*
 *  Context.mm
 *  Dream
 *
 *  Created by Samuel Williams on 20/04/09.
 *  Copyright 2009 Orion Transfer Ltd. All rights reserved.
 *
 */

#include "Context.h"
#include "DreamView.h"
#include "DreamApplicationDelegate.h"

namespace Dream
{
	namespace Client
	{
		namespace Display
		{
			void IApplication::start (PTR(IApplication) application, PTR(Dictionary) config)
			{
				UIKitContext::setApplicationInstance(application);
				
				[DreamApplicationDelegate start];
			}
			
			namespace UIKitContext
			{
				static IInputHandler * g_inputHandler;
				
				IInputHandler * globalInputHandler ()
				{
					return g_inputHandler;
				}
				
				static REF(IApplication) g_applicationInstance;
				
				void setApplicationInstance (REF(IApplication) application)
				{
					g_applicationInstance = application;
				}
				
				void runApplicationCallback ()
				{
					g_applicationInstance->run();
				}
				
#pragma mark -
#pragma mark Renderer Subclass
				
				typedef OpenGLES11::Renderer TouchRenderer;
				
#pragma mark -
				
				
				
				struct Context::ContextImpl {
					ContextImpl () {}
					
					REF(TouchRenderer) renderer;
					
					UIWindow * window;
					EAGLView * view;
										
					FrameCallbackT frameCallback;
				};
								
				void Context::setTitle (String title) {
					
				}
				
				void Context::show () {
					if (!m_impl) {
						m_impl = new ContextImpl;
						
						// Create window
						m_impl->window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
						
						EAGLViewOpenGLVersion version = OPENGLES_11;
						
						bool enableShaders = false;
						m_config->get("OpenGL.EnableShaders", enableShaders);
						
						if (enableShaders) {
							std::cerr << "Requesting OpenGL Shaders..." << std::endl;
							version = OPENGLES_20;
						}
						
						// Set up content view
						CGRect frame = [[UIScreen mainScreen] applicationFrame];
						m_impl->view = [[[DreamView alloc] initWithFrame:frame version:version] autorelease];
						[m_impl->window addSubview:m_impl->view];
						
						m_impl->renderer = new TouchRenderer();
						
						std::cerr << "OpenGL Context Initialized..." << std::endl;
						std::cerr << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
						std::cerr << "OpenGL Renderer: " << glGetString(GL_RENDERER) << " " << glGetString(GL_VERSION) << std::endl;
						
						glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
						glPixelStorei(GL_PACK_ALIGNMENT, 1);
						
						//[m_impl->view beginDrawing];
					}
				
					[m_impl->window makeKeyAndVisible];
				}
				
				void Context::hide () {
					// N/A
				}
				
				void Context::setFrameSync (bool vsync) {
					//GLint swapInt = vsync ? 1 : 0;
					//[[[m_impl->window contentView] openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
				}
				
				Context::Context (PTR(Dictionary) config) : m_impl(NULL), m_config(config) {
					
				}
				
				Context::~Context () {
					[m_impl->window release];
					
					delete m_impl;
				}
				
				ResolutionT Context::resolution ()
				{
					CGSize size = [m_impl->view frame].size;
					
					return ResolutionT(size.width, size.height);
				}
				
				const char * getSymbolicError (GLenum error) {
					switch (error) {
						case GL_NO_ERROR:
							return "No error has occurred.";
						case GL_INVALID_ENUM:
							return "An invalid value has been specified (GL_INVALID_ENUM)!";
						case GL_INVALID_VALUE:
							return "A numeric argument is out of range (GL_INVALID_VALUE)!";
						case GL_INVALID_OPERATION:
							return "The specified operation is not allowed (GL_INVALID_OPERATION)!";
						case GL_STACK_OVERFLOW:
							return "The specified command would cause a stack overflow (GL_STACK_OVERFLOW)!";
						case GL_STACK_UNDERFLOW:
							return "The specified command would cause a stack underflow (GL_STACK_UNDERFLOW)!";
						case GL_OUT_OF_MEMORY:
							return "There is not enough free memory left to run the command (GL_OUT_OF_MEMORY)!";
						default:
							return "An unknown error has occurred!";
					}
				}
				
				void Context::flipBuffers ()
				{
					GLenum error;
					
					while ((error = glGetError()) != GL_NO_ERROR)
						std::cerr << "OpenGL Error " << "#" << error << ": " << getSymbolicError(error) << std::endl;
					
					[m_impl->view flipBuffers];
				}
				
				void Context::processPendingEvents (IInputHandler * handler)
				{
					g_inputHandler = handler;
					
					SInt32 result;
					
					do {
						result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, TRUE);
					} while (result == kCFRunLoopRunHandledSource);
					
					g_inputHandler = NULL;
				}
				
				void Context::scheduleFrameNotificationCallback (REF(Events::Loop) loop, FrameCallbackT callback)
				{
					if (!callback && m_timerSource) {
						m_timerSource->cancel();
						m_timerSource = NULL;
					}
					
					if (callback) {
						m_timerSource = new FrameTimerSource(callback, 1.0/30.0);
						loop->scheduleTimer(m_timerSource);
					}
				}
				
				REF(TouchRenderer) Context::renderer ()
				{
					return m_impl->renderer;
				}
				
			}
		}
	}
}
