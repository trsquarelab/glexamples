package com.example.gles_3_helloworld;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.util.Log;

public class GLES3View extends GLSurfaceView implements GLSurfaceView.Renderer {

	private static final String TAG = "GLES_3_HELLOWORLD";
	
	private static final String VertexShader = 
			"#version 300 es                               \n" +
			"layout(location = 0) in vec4 aPosition;       \n" +
			"void main()                                   \n" +
			"{                                             \n" +
			"    gl_Position = aPosition;                  \n" +
			"}                                             \n" +
			"                                              \n";
			
	private static final String FragmentShader = 
			"#version 300 es                               \n" +
		    "precision mediump float;                      \n" +
		    "out vec4 fragColor;                           \n" +
			"void main()                                   \n" +
		    "{                                             \n" +
			"    fragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f); \n" +
			"}                                             \n" +
			"";
	
	
	final int mPosLoc = 0;

	int mProgram;

	FloatBuffer mVerticesBuffer;

	public GLES3View(Context context) {
		super(context);
		
		// setup EGL configurations
		setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        setEGLContextClientVersion(2);
        
        setRenderer(this);
	}

	private void init() {
		GLES30.glClearColor(0.5f, 0.5f, 0.5f, 1f);
		
		mProgram = createProgram(VertexShader, FragmentShader);
		
		// vertices
		float vertices[] = {
				-0.75f, -0.75f,
				0.75f, -0.75f,
				0.75f, 0.75f,
				-0.75f, 0.75f				
		};
		
		// create the float buffer
		ByteBuffer vbb = ByteBuffer.allocateDirect(vertices.length * 4);
		vbb.order(ByteOrder.nativeOrder()); 
		mVerticesBuffer = vbb.asFloatBuffer();
		mVerticesBuffer.put(vertices);
		mVerticesBuffer.position(0);
	}

	private void draw() {
		GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT);
		
		GLES30.glUseProgram(mProgram);
		
		GLES30.glVertexAttribPointer(mPosLoc, 2, GLES30.GL_FLOAT, false, 0, mVerticesBuffer);
		GLES30.glEnableVertexAttribArray(mPosLoc);
		
		GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 0, 4);
	}

	private int loadShader(int shaderType, String source) {
		Log.d(TAG, source);
		
	    int shader = GLES30.glCreateShader(shaderType);
	    if (shader != 0) {
	    	// compile the shader
	    	GLES30.glShaderSource(shader, source);
	    	GLES30.glCompileShader(shader);
	    	
	        int[] compiled = new int[1];
	        GLES30.glGetShaderiv(shader, GLES30.GL_COMPILE_STATUS, compiled, 0);
	        if (compiled[0] == 0) {
	            Log.e(TAG, GLES30.glGetShaderInfoLog(shader));
                GLES30.glDeleteShader(shader);
                shader = 0;
	        }
	    }
	    
	    return shader;
	}

	private int createProgram(String vertexSource, String fragmentSource) {
	    int vertexShader = loadShader(GLES30.GL_VERTEX_SHADER, vertexSource);
	    if (vertexShader == 0) {
	        return 0;
	    }

	    int pixelShader = loadShader(GLES30.GL_FRAGMENT_SHADER, fragmentSource);
	    if (pixelShader == 0) {
	        return 0;
	    }

	    int program = GLES30.glCreateProgram();
	    if (program != 0) {
	    	GLES30.glAttachShader(program, vertexShader);
	    	GLES30.glAttachShader(program, pixelShader);
	    	GLES30.glLinkProgram(program);
	        int []linkStatus = {0};
	        GLES30.glGetProgramiv(program, GLES30.GL_LINK_STATUS, linkStatus, 0);
	        if (linkStatus[0] != 1) {
	            Log.e(TAG, GLES30.glGetProgramInfoLog(program));
	            GLES30.glDeleteProgram(program);
	            program = 0;
	        }
	    }
	    return program;
	}
	
	
	
    public void onDrawFrame(GL10 gl) {
    	draw();
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES30.glViewport(0, 0, width, height);
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    	init();
    }
	
}
