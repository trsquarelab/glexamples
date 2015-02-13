
#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <iostream>

#include "rcommon.h"

namespace chikku
{

class RScene
{
public:
    static RScene * create(int argc, char **argv)
    {
    	glutInit(&argc, argv);
    	
        if (RScene::instance().init()) {
            return &RScene::instance();
        }
        return 0;
    }

    int exec()
    {
        glutMainLoop();
        return 1;
    }

private:
    static void reshape(int w, int h)
    {
    	RScene::instance().mW = w;
    	RScene::instance().mH = h;
        RScene::instance().onReshape(w, h);
    }

    static void display()
    {
        RScene::instance().onDisplay();
    }

    static void mouse(int button, int state, int x, int y)
    {
        RScene::instance().onMouseEvent(button, state, x, y);
    }

    static void menu(int id)
    {
        RScene::instance().onMenuItemClicked(id);
    }
    
    static void timer(int)
    {
        RScene::instance().onTimer();
    }
    
    static RScene& instance();
    
protected:
    RScene(std::string const & title)
     : mW(chikku::WindowWidth),
       mH(chikku::WindowHeight),
       mX(chikku::WindowPositionX),
       mY(chikku::WindowPositionY),
       mTitle(title)
    {}

    void addTimer(unsigned int delay = chikku::TimerDelay)
    {
    	glutTimerFunc(delay, RScene::timer, 0);
    }
    
    virtual bool onInit()
    {
    	return true;
    }
    
    virtual void onMenuItemClicked(int /*id*/)
    {}

    virtual void onReshape(int /*w*/, int /*h*/)
    {}

    virtual void onMouseEvent(int /*button*/, int /*state*/, int /*x*/, int /*y*/)
    {}

    virtual void onDisplay() const = 0;

    virtual void onTimer()
    {}
    
    void redraw()
    {
        glutPostRedisplay();
    }

protected:
    virtual bool init()
    {
        glutInitWindowPosition(mX, mY);
        glutInitWindowSize(mW, mH);
        glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | 
        					GLUT_DOUBLE | GLUT_DEPTH);

        glutCreateWindow(mTitle.c_str());

        glutReshapeFunc(RScene::reshape);
        glutDisplayFunc(RScene::display);
        glutMouseFunc(RScene::mouse);
        glutTimerFunc(chikku::TimerDelay, RScene::timer, 0);
        
        mMenu = glutCreateMenu(RScene::menu);
        glutAttachMenu(GLUT_RIGHT_BUTTON);
        
        std::cout << "VENDOR : "<< glGetString(GL_VENDOR) << std::endl;
        std::cout << "RENDERER : " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "VERSION : " << glGetString(GL_VERSION) << std::endl;
        //std::cout << "EXTENSIONS : " << glGetString(GL_EXTENSIONS) << std::endl;

        if (!onInit()) {
        	std::cerr << "initialization failed" << std::endl;
        	return false;
        }
        
        return true;
    }

protected:
    int mW;
    int mH;
    int mX;
    int mY;
    std::string mTitle;
    int mMenu;
};

}

#endif
