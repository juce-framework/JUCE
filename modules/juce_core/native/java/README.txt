The java code in the module's native/java subfolders were used to generate dex byte-code in various places in the JUCE framework. These are the steps required to re-generate the dex byte-code from any java source code inside the native/java subfolders:

1. Create a new JUCE android project with the minimal sdk version which is required for the java source code you wish to compile.
2. Move the the .java files that you wish to create source code for, into the module's native/javacore/app folder. Remember that .java files need to be in nested sub-folders which resemble their package, i.e. a Java class com.roli.juce.HelloWorld.java should be in the module's native/javacore/app/com/roli/juce folder.
3. Build your project with AS and run. The app will now use the source code in the folder creates in step 2 - so you can debug your java code this way.
4. Once everything is working, rebuild your app in release mode.
5. Go to your app's Builds/Android folder. Inside there you will find app/build/intermediates/classes/release_/release. Inside of that folder, you will find all your java byte-code compiled classes. Remove any classes that you are not interested in (typically you'll find Java.class, JuceApp.class and JuceSharingContentProvider.class which you will probably want to remove).
6. Inside of app/build/intermediates/classes/release_/release execute the following dx command:

   <path-to-your-android-sdk>/build-tools/<latest-build-tool-version>/dx --dex --verbose --min-sdk-version=<your-min-sdk-of-your-classes> --output /tmp/JavaDexByteCode.dex .


   Replace <your-min-sdk-of-your-classes> with the minimal sdk version you used in step 1.
   
7. gzip the output:

gzip /tmp/JavaDexByteCode.dex

8. The output /tmp/JavaDexByteCode.dex.gz is now the byte code that can be included into JUCE. You can use the Projucer's BinaryData generator functionality to get this into a convenient char array like form.
