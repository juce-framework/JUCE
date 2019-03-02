#if UNITY_EDITOR

using UnityEditor;
using UnityEngine;

using System.Collections.Generic;
using System.Runtime.InteropServices;

public class %%plugin_class_name%%GUI : IAudioEffectPluginGUI
{
    public override string Name           { get { return "%%plugin_name%%"; } }
    public override string Description    { get { return "%%plugin_description%%"; } }
    public override string Vendor         { get { return "%%plugin_vendor%%"; } }

    //==============================================================================
	[DllImport("%%plugin_name%%")] static extern System.IntPtr getRenderCallback();

    [DllImport("%%plugin_name%%")] static extern void unityInitialiseTexture (int id, System.IntPtr texture, int width, int height);

    [DllImport("%%plugin_name%%")] static extern void unityMouseDown (int id, float x, float y, EventModifiers mods, int button);
    [DllImport("%%plugin_name%%")] static extern void unityMouseDrag (int id, float x, float y, EventModifiers mods, int button);
    [DllImport("%%plugin_name%%")] static extern void unityMouseUp   (int id, float x, float y, EventModifiers mods);

    [DllImport("%%plugin_name%%")] static extern void unityKeyEvent (int id, KeyCode code, EventModifiers mods, string name);

    [DllImport("%%plugin_name%%")] static extern void unitySetScreenBounds (int id, float x, float y, float w, float h);

    //==============================================================================
    private class PluginGUIInstance
    {
        public PluginGUIInstance (ref IAudioEffectPlugin plugin, int id)
        {
            instanceID = id;

            float[] arr;
            plugin.GetFloatBuffer ("Editor", out arr, 1);
            hasEditor = (arr[0] > 0.0f);
        }

        public void repaint (Rect r)
        { 
            Vector2 newScreenPosition = GUIUtility.GUIToScreenPoint (r.position);

            if (bounds != r 
                || screenPosition != newScreenPosition)
            {
                screenPosition = newScreenPosition;
                bounds = r;

                unitySetScreenBounds (instanceID, screenPosition.x, screenPosition.y, bounds.width, bounds.height);
                setupTexture();
            }

			GL.IssuePluginEvent (getRenderCallback(), instanceID);

            texture.SetPixels32 (pixels);
            texture.Apply();

            EditorGUI.DrawPreviewTexture (bounds, texture);
        }

        public bool handleMouseEvent (EventType eventType)
        {
            Vector2 mousePos = Event.current.mousePosition;
            EventModifiers mods = Event.current.modifiers;

            if (! bounds.Contains (mousePos))
                return false;

            Vector2 relativePos = new Vector2 (mousePos.x - bounds.x, mousePos.y - bounds.y);

            if (eventType == EventType.MouseDown)    
            {
                unityMouseDown (instanceID, relativePos.x, relativePos.y, mods, Event.current.button);
                GUIUtility.hotControl = GUIUtility.GetControlID (FocusType.Passive);
            }
            else if (eventType == EventType.MouseUp)
            {
                unityMouseUp (instanceID, relativePos.x, relativePos.y, mods);
                GUIUtility.hotControl = 0;
            }
            else if (eventType == EventType.MouseDrag)    
            {
                unityMouseDrag (instanceID, relativePos.x, relativePos.y, mods, Event.current.button);
            }

            Event.current.Use();

            return true;
        }

        public void handleKeyEvent (EventType eventType)
        {
            if (eventType == EventType.KeyDown)
            {
                KeyCode code = Event.current.keyCode;

                if (code == KeyCode.None)
                    return;

                EventModifiers mods = Event.current.modifiers;

                unityKeyEvent (instanceID, code, mods, code.ToString());
            }
        }

        private void setupTexture()
        {
            if (pixelHandle.IsAllocated)
                pixelHandle.Free();

            texture = new Texture2D ((int) bounds.width, (int) bounds.height, TextureFormat.ARGB32, false);

            pixels = texture.GetPixels32();
            pixelHandle = GCHandle.Alloc (pixels, GCHandleType.Pinned);

            unityInitialiseTexture (instanceID, pixelHandle.AddrOfPinnedObject(), texture.width, texture.height);
        }

        public int instanceID = -1;
        public bool hasEditor;

        private Vector2 screenPosition;
        private Rect bounds;

        private Texture2D texture;
        private Color32[] pixels;
        private GCHandle pixelHandle;
    }
    List<PluginGUIInstance> guis = new List<PluginGUIInstance>();

    private PluginGUIInstance getGUIInstanceForPlugin (ref IAudioEffectPlugin plugin)
    {
        float[] idArray;
        plugin.GetFloatBuffer ("ID", out idArray, 1);

        int id = (int) idArray[0];

        for (int i = 0; i < guis.Count; ++i)
        {
            if (guis[i].instanceID == id)
                return guis[i];
        }

        PluginGUIInstance newInstance = new PluginGUIInstance (ref plugin, id);
        guis.Add (newInstance);

        return guis[guis.Count - 1];
    }

    //==============================================================================
    public override bool OnGUI (IAudioEffectPlugin plugin)
    {
        PluginGUIInstance guiInstance = getGUIInstanceForPlugin (ref plugin);

        if (! guiInstance.hasEditor)
            return true;

        float[] arr;
        plugin.GetFloatBuffer ("Size", out arr, 6);

        Rect r = GUILayoutUtility.GetRect (arr[0], arr[1],
                                           new GUILayoutOption[] { GUILayout.MinWidth (arr[2]), GUILayout.MinHeight (arr[3]),
                                                                   GUILayout.MaxWidth (arr[4]), GUILayout.MaxHeight (arr[5]) });

        int controlID = GUIUtility.GetControlID (FocusType.Passive);
        Event currentEvent = Event.current;
        EventType currentEventType = currentEvent.GetTypeForControl (controlID);

        if (currentEventType == EventType.Repaint)
            guiInstance.repaint (r);
        else if (currentEvent.isMouse)
            guiInstance.handleMouseEvent (currentEventType);
        else if (currentEvent.isKey)
            guiInstance.handleKeyEvent (currentEventType);

        return false;
    }
}

#endif
