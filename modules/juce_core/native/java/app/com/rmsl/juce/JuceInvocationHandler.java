package com.rmsl.juce;

import java.lang.reflect.*;

public class JuceInvocationHandler implements InvocationHandler
{
        public JuceInvocationHandler (long nativeContextRef)
        {
                nativeContext = nativeContextRef;
        }

        public void clear()
        {
                nativeContext = 0;
        }

        @Override
        public void finalize()
        {
                if (nativeContext != 0)
                        dispatchFinalize (nativeContext);
        }

        @Override
        public Object invoke (Object proxy, Method method, Object[] args) throws Throwable
        {
                if (nativeContext != 0)
                        return dispatchInvoke (nativeContext, proxy, method, args);

                return null;
        }

        //==============================================================================
        private long nativeContext = 0;

        private native void dispatchFinalize (long nativeContextRef);
        private native Object dispatchInvoke (long nativeContextRef, Object proxy, Method method, Object[] args);
}
