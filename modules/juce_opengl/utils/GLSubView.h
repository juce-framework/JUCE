
#pragma once

class DummyView:public Component{
public:
    void paint(Graphics& g) override{
        g.fillAll(Colours::white);
    }
};

class GLSubFrame:private Thread {

public:
    class Renderer{
    public:
        virtual ~Renderer(){};
        virtual void initGL() = 0;
        virtual void renderGL() = 0;
        virtual void closeGL() = 0;
    };
public:
    GLSubFrame(Renderer* r,Component *c,Component* focus);
    ~GLSubFrame();
    
    virtual void run();
    
    void triggerRepaint();
    
    const OpenGLContext& getGLContext()const {return mContext;}
    OpenGLContext& getGLContext() {return mContext;}

    
private:
    ScopedPointer<OpenGLContext::NativeContext> mNativeContext;
    Atomic<int> needsUpdate;
    Rectangle<int> viewportArea, lastScreenBounds;
    double scale;
    
private:
    bool hasInitialised;
    void start();
    void stop();
    void initOnThread();
    void shutdownOnThread();
    bool renderFrame();
    void updateViewportSize();
    
    friend class GLSubView;
    
protected:
    OpenGLContext mContext;
    Renderer* mRenderer;
    Component *mComp;
    Component *mFocus;
    
};

//----------------------------------------------GLSubView-------------------------------------------------------------------------

class GLSubView:public Component, public GLSubFrame::Renderer{
    
public:
    
    GLSubView(int x,int y,int w,int h);
    virtual ~GLSubView();
    void paint(Graphics &g) override final;
    const OpenGLContext& getGLContext()const {return mSubFrame->getGLContext();}
    void shutdownOpenGL();
    void triggerRepaint();
    Image getImage();
    static void verticalRowFlip(PixelARGB*const data,int w, int h);

public:
    virtual void openGLCreated() = 0;
    virtual void openGLClosing() = 0;
    virtual void openGLRender() = 0;
    virtual void paintOver(Graphics& g){};

private:
    std::unique_ptr<DummyView> mDummyComp;
    std::unique_ptr<GLSubFrame> mSubFrame;
    OpenGLFrameBuffer* mFrameBuffer;
    GLuint fbo;
    GLuint depthOrStencilBuffer;
    GLuint ColorBufferID;
    double mScale;

    std::mutex mutex;
    PixelARGB* mPixels;
    int mWidth;
    int mHeight;
    std::unique_ptr<Image> mImage;
    
private:
    void initGL()override final;
    void renderGL()override final;
    void closeGL() override final;
    
};




