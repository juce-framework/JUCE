/* Copyright 2016 nTopology Inc. All Rights Reserved.
GLSubView.cpp
Author: Abhi
==============================================================================*/

//--GLSubFrame------------------------------------------------------------------
//
GLSubFrame::GLSubFrame(Renderer* r,Component *attach,Component* focus):
  Thread ("OpenGLSub Rendering")
{

    
    hasInitialised = false;
    needsUpdate = 1;
    mRenderer = r;
    
    mComp = attach;
    mFocus = focus;
    
    auto pixFormat = OpenGLPixelFormat{};
    pixFormat.multisamplingLevel = 0;
    mContext.setPixelFormat(pixFormat);
    mContext.setMultisamplingEnabled(true);
    mContext.setContinuousRepainting(false);
    mContext.setOpenGLVersionRequired(OpenGLContext::openGL3_2);
    
    mNativeContext= nullptr;
    mNativeContext = new OpenGLContext::NativeContext (*mComp,
                                                       mContext.openGLPixelFormat, 
                                                       mContext.contextToShareWith,
                                                       mContext.useMultisampling,
                                                       mContext.versionRequired);
    
    if (mNativeContext->createdOk())
        mContext.nativeContext = mNativeContext;
    else
        mNativeContext = nullptr;
    
	updateViewportSize();
	mContext.currentRenderScale = scale;

    start();
}

void GLSubFrame::run(){
    {
        // Allow the message thread to finish setting-up the context before using it..
        MessageManagerLock mml (this);
        if (! mml.lockWasGained())
            return;
    }
    initOnThread();
    hasInitialised = true;
    while (! threadShouldExit())
    {
        if (! renderFrame())
            wait (5); // failed to render, so avoid a tight fail-loop.
        else if (! mContext.continuousRepaint)
            wait (-1);
    }
    
    shutdownOnThread();  
}

GLSubFrame::~GLSubFrame(){
    stop();
}

void GLSubFrame::start(){
    if (mNativeContext != nullptr)
        startThread (6);
}

void GLSubFrame::stop(){
    stopThread (10000);
    mContext.nativeContext = nullptr;
    hasInitialised = false;
}

void GLSubFrame::initOnThread(){
    mContext.makeActive();
    mNativeContext->initialiseOnRenderThread(mContext);
    mContext.extensions.initialise();
    mNativeContext->setSwapInterval (1);
    MessageManagerLock mml (Thread::getCurrentThread());
    mRenderer->initGL();
}

bool GLSubFrame::renderFrame(){

    if (! mContext.makeActive()){
        return false;
    }
    
//    {
//    MessageManagerLock mml (Thread::getCurrentThread());
//    if (! mml.lockWasGained())
//        return false;
//    updateViewportSize();
//    }
    
    OpenGLContext::NativeContext::Locker locker (*mNativeContext);
    mContext.currentRenderScale = scale;
    if(!mRenderer->renderGL())return false;
    //mContext.swapBuffers();
    
    return true;
}

void GLSubFrame::updateViewportSize ()
{
   // if (ComponentPeer* peer = mFocus->getPeer())
    //{
        lastScreenBounds = mFocus->getTopLevelComponent()->getScreenBounds();
        
        const double newScale = Desktop::getInstance().getDisplays()
        .getDisplayContaining (lastScreenBounds.getCentre()).scale;
        
//        Rectangle<int> newArea (peer->getComponent().getLocalArea (mFocus, mFocus->getLocalBounds())
//                                .withZeroOrigin()
//                                * newScale);
        
        if (scale != newScale)
        {
            scale = newScale;
            
        }
    //}
}

void GLSubFrame::shutdownOnThread(){
    mRenderer->closeGL();
    mNativeContext->shutdownOnRenderThread();
}

void GLSubFrame::triggerRepaint(){
    notify();
}

//--GLSubView-------------------------------------------------------------------
//
GLSubView::GLSubView(int x,int y,int w,int h){
    //find desktop scale
    
    auto lastScreenBounds = getTopLevelComponent()->getScreenBounds();
    mScale = Desktop::getInstance().getDisplays().getDisplayContaining (lastScreenBounds.getCentre()).scale;
    mWidth = int(w * mScale);
    mHeight = int(h * mScale);
    
    setRepaintsOnMouseActivity(false);

    
    mPixels = new PixelARGB[mWidth * mHeight];
    mDummyComp = std::make_unique<DummyView>();
    mDummyComp->setBounds(0,0,w,h);
    setBounds(x, y, w, h);
    mFrameBuffer = new OpenGLFrameBuffer();
    mImage = std::make_unique<Image>(Image::ARGB,mWidth,mHeight,true);
    mSubFrame = std::make_unique<GLSubFrame>(this,mDummyComp.get(),this);
}

void GLSubView::shutdownOpenGL(){
    mSubFrame->stop();
}

GLSubView::~GLSubView(){
    delete[] mPixels;
}

void GLSubView::initGL(){
    auto& context = getGLContext();
    mFrameBuffer->initialise(mSubFrame->getGLContext(), mWidth, mHeight);
    
    const GLsizei nSamples = 4;
    
    context.extensions.glGenFramebuffers( 1, &fbo );
    context.extensions.glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    
    context.extensions.glGenRenderbuffers(1, &ColorBufferID);
    context.extensions.glBindRenderbuffer(GL_RENDERBUFFER, ColorBufferID);
    context.extensions.glRenderbufferStorageMultisample(GL_RENDERBUFFER, 
                                                        nSamples, 
                                                        GL_RGBA8,
                                                        mWidth, 
                                                        mHeight);
    
    context.extensions.glGenRenderbuffers (1, &depthOrStencilBuffer);
    context.extensions.glBindRenderbuffer (GL_RENDERBUFFER,
                                           depthOrStencilBuffer);

    context.extensions.glRenderbufferStorageMultisample (GL_RENDERBUFFER,
                                                         nSamples, 
                                                         GL_DEPTH_COMPONENT24,
                                                         mWidth,mHeight);
    
    context.extensions.glFramebufferRenderbuffer (GL_FRAMEBUFFER, 
                                                  GL_COLOR_ATTACHMENT0, 
                                                  GL_RENDERBUFFER,
                                                  ColorBufferID);

    context.extensions.glFramebufferRenderbuffer (GL_FRAMEBUFFER,
                                                  GL_DEPTH_ATTACHMENT, 
                                                  GL_RENDERBUFFER, 
                                                  depthOrStencilBuffer);
    
    openGLCreated();
}

void GLSubView::verticalRowFlip(PixelARGB*const data,int w, int h){
    HeapBlock<PixelARGB> tempRow ((size_t) w);
    const size_t rowSize = sizeof (PixelARGB) * (size_t) w;
    
    for (int y = 0; y < h / 2; ++y)
    {
        PixelARGB* const row1 = data + y * w;
        PixelARGB* const row2 = data + (h - 1 - y) * w;
        memcpy (tempRow, row1, rowSize);
        memcpy (row1, row2, rowSize);
        memcpy (row2, tempRow, rowSize);
    }
}

bool GLSubView::renderGL(){
    auto& context = getGLContext();

    context.extensions.glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    openGLRender();
    context.extensions.glBindFramebuffer(GL_FRAMEBUFFER,0);
    
    //blit
    context.extensions.glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    context.extensions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFrameBuffer->getFrameBufferID());
    context.extensions.glBlitFramebuffer(0,0,
                                         mWidth, mHeight, 
                                         0,0,
                                         mWidth, mHeight, 
                                         GL_COLOR_BUFFER_BIT, 
                                         GL_NEAREST);
    
    std::unique_lock<std::mutex> lock(mutex);
    mFrameBuffer->readPixels(mPixels, Rectangle<int>{0,0,mWidth,mHeight});
    lock.unlock();
    
    MessageManagerLock mmLock(Thread::getCurrentThread());
    if(!mmLock.lockWasGained())return false;
    repaint();
    return true;
}

void GLSubView::closeGL(){
    auto& context = getGLContext();
    delete mFrameBuffer;
    context.extensions.glDeleteFramebuffers(1,&fbo);
    context.extensions.glDeleteRenderbuffers (1, &depthOrStencilBuffer);
    context.extensions.glDeleteRenderbuffers (1, &ColorBufferID);

    openGLClosing();
}

void GLSubView::paint(juce::Graphics &g){
    std::unique_lock<std::mutex> lock(mutex);
    Image::BitmapData bitmap(*mImage,0,0,mWidth,mHeight,Image::BitmapData::writeOnly);
    memcpy(bitmap.data, mPixels, sizeof(uint8) * 4 * mWidth * mHeight);
    auto affine = AffineTransform::verticalFlip(float(getHeight()));
    g.addTransform(affine);
    g.drawImage(*mImage, 0, 0,getWidth(),getHeight(),0,0,mWidth,mHeight,false);
    lock.unlock();
    paintOver(g);
}

void GLSubView::triggerRepaint(){
    mSubFrame->triggerRepaint();
}

Image GLSubView::getImage(){
    mutex.lock();
    auto copy = mImage->createCopy();
    mutex.unlock();
    return copy;
}

