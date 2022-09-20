/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#define JUCE_NATIVE_ACCESSIBILITY_INCLUDED 1

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (setSource,                "setSource",                "(Landroid/view/View;I)V") \
 METHOD (addChild,                 "addChild",                 "(Landroid/view/View;I)V") \
 METHOD (setParent,                "setParent",                "(Landroid/view/View;)V") \
 METHOD (setVirtualParent,         "setParent",                "(Landroid/view/View;I)V") \
 METHOD (setBoundsInScreen,        "setBoundsInScreen",        "(Landroid/graphics/Rect;)V") \
 METHOD (setBoundsInParent,        "setBoundsInParent",        "(Landroid/graphics/Rect;)V") \
 METHOD (setPackageName,           "setPackageName",           "(Ljava/lang/CharSequence;)V") \
 METHOD (setClassName,             "setClassName",             "(Ljava/lang/CharSequence;)V") \
 METHOD (setContentDescription,    "setContentDescription",    "(Ljava/lang/CharSequence;)V") \
 METHOD (setCheckable,             "setCheckable",             "(Z)V") \
 METHOD (setChecked,               "setChecked",               "(Z)V") \
 METHOD (setClickable,             "setClickable",             "(Z)V") \
 METHOD (setEnabled,               "setEnabled",               "(Z)V") \
 METHOD (setFocusable,             "setFocusable",             "(Z)V") \
 METHOD (setFocused,               "setFocused",               "(Z)V") \
 METHOD (setPassword,              "setPassword",              "(Z)V") \
 METHOD (setSelected,              "setSelected",              "(Z)V") \
 METHOD (setVisibleToUser,         "setVisibleToUser",         "(Z)V") \
 METHOD (setAccessibilityFocused,  "setAccessibilityFocused",  "(Z)V") \
 METHOD (setText,                  "setText",                  "(Ljava/lang/CharSequence;)V") \
 METHOD (setMovementGranularities, "setMovementGranularities", "(I)V") \
 METHOD (addAction,                "addAction",                "(I)V") \

 DECLARE_JNI_CLASS (AndroidAccessibilityNodeInfo, "android/view/accessibility/AccessibilityNodeInfo")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (setCollectionInfo, "setCollectionInfo", "(Landroid/view/accessibility/AccessibilityNodeInfo$CollectionInfo;)V") \
 METHOD (setCollectionItemInfo, "setCollectionItemInfo", "(Landroid/view/accessibility/AccessibilityNodeInfo$CollectionItemInfo;)V")

 DECLARE_JNI_CLASS (AndroidAccessibilityNodeInfo19, "android/view/accessibility/AccessibilityNodeInfo")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (obtain, "obtain", "(IIZ)Landroid/view/accessibility/AccessibilityNodeInfo$CollectionInfo;")

 DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidAccessibilityNodeInfoCollectionInfo, "android/view/accessibility/AccessibilityNodeInfo$CollectionInfo", 19)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (obtain, "obtain", "(IIIIZ)Landroid/view/accessibility/AccessibilityNodeInfo$CollectionItemInfo;")

 DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidAccessibilityNodeInfoCollectionItemInfo, "android/view/accessibility/AccessibilityNodeInfo$CollectionItemInfo", 19)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (obtain, "obtain", "(I)Landroid/view/accessibility/AccessibilityEvent;") \
 METHOD (setPackageName, "setPackageName", "(Ljava/lang/CharSequence;)V") \
 METHOD (setSource, "setSource","(Landroid/view/View;I)V") \
 METHOD (setAction, "setAction", "(I)V") \
 METHOD (setFromIndex, "setFromIndex", "(I)V") \
 METHOD (setToIndex, "setToIndex", "(I)V") \

 DECLARE_JNI_CLASS (AndroidAccessibilityEvent, "android/view/accessibility/AccessibilityEvent")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (isEnabled, "isEnabled", "()Z") \

 DECLARE_JNI_CLASS (AndroidAccessibilityManager, "android/view/accessibility/AccessibilityManager")
#undef JNI_CLASS_MEMBERS

namespace
{
    constexpr int HOST_VIEW_ID = -1;

    constexpr int TYPE_VIEW_CLICKED                                = 0x00000001,
                  TYPE_VIEW_SELECTED                               = 0x00000004,
                  TYPE_VIEW_ACCESSIBILITY_FOCUSED                  = 0x00008000,
                  TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED            = 0x00010000,
                  TYPE_WINDOW_CONTENT_CHANGED                      = 0x00000800,
                  TYPE_VIEW_TEXT_SELECTION_CHANGED                 = 0x00002000,
                  TYPE_VIEW_TEXT_CHANGED                           = 0x00000010,
                  TYPE_VIEW_TEXT_TRAVERSED_AT_MOVEMENT_GRANULARITY = 0x00020000;

    constexpr int CONTENT_CHANGE_TYPE_SUBTREE             = 0x00000001,
                  CONTENT_CHANGE_TYPE_CONTENT_DESCRIPTION = 0x00000004;

    constexpr int ACTION_ACCESSIBILITY_FOCUS              = 0x00000040,
                  ACTION_CLEAR_ACCESSIBILITY_FOCUS        = 0x00000080,
                  ACTION_CLEAR_FOCUS                      = 0x00000002,
                  ACTION_CLEAR_SELECTION                  = 0x00000008,
                  ACTION_CLICK                            = 0x00000010,
                  ACTION_COLLAPSE                         = 0x00080000,
                  ACTION_EXPAND                           = 0x00040000,
                  ACTION_FOCUS                            = 0x00000001,
                  ACTION_NEXT_AT_MOVEMENT_GRANULARITY     = 0x00000100,
                  ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY = 0x00000200,
                  ACTION_SCROLL_BACKWARD                  = 0x00002000,
                  ACTION_SCROLL_FORWARD                   = 0x00001000,
                  ACTION_SELECT                           = 0x00000004,
                  ACTION_SET_SELECTION                    = 0x00020000,
                  ACTION_SET_TEXT                         = 0x00200000;

    constexpr int MOVEMENT_GRANULARITY_CHARACTER = 0x00000001,
                  MOVEMENT_GRANULARITY_LINE      = 0x00000004,
                  MOVEMENT_GRANULARITY_PAGE      = 0x00000010,
                  MOVEMENT_GRANULARITY_PARAGRAPH = 0x00000008,
                  MOVEMENT_GRANULARITY_WORD      = 0x00000002,
                  ALL_GRANULARITIES = MOVEMENT_GRANULARITY_CHARACTER
                                    | MOVEMENT_GRANULARITY_LINE
                                    | MOVEMENT_GRANULARITY_PAGE
                                    | MOVEMENT_GRANULARITY_PARAGRAPH
                                    | MOVEMENT_GRANULARITY_WORD;

    constexpr int ACCESSIBILITY_LIVE_REGION_POLITE = 0x00000001;
}

static jmethodID nodeInfoSetEditable                     = nullptr;
static jmethodID nodeInfoSetTextSelection                = nullptr;
static jmethodID nodeInfoSetLiveRegion                   = nullptr;
static jmethodID accessibilityEventSetContentChangeTypes = nullptr;

template <typename MemberFn>
static AccessibilityHandler* getEnclosingHandlerWithInterface (AccessibilityHandler* handler, MemberFn fn)
{
    if (handler == nullptr)
        return nullptr;

    if ((handler->*fn)() != nullptr)
        return handler;

    return getEnclosingHandlerWithInterface (handler->getParent(), fn);
}

static void loadSDKDependentMethods()
{
    static bool hasChecked = false;

    if (! hasChecked)
    {
        hasChecked = true;

        auto* env = getEnv();
        const auto sdkVersion = getAndroidSDKVersion();

        if (sdkVersion >= 18)
        {
            nodeInfoSetEditable      = env->GetMethodID (AndroidAccessibilityNodeInfo, "setEditable",      "(Z)V");
            nodeInfoSetTextSelection = env->GetMethodID (AndroidAccessibilityNodeInfo, "setTextSelection", "(II)V");
        }

        if (sdkVersion >= 19)
        {
            nodeInfoSetLiveRegion                   = env->GetMethodID (AndroidAccessibilityNodeInfo, "setLiveRegion",         "(I)V");
            accessibilityEventSetContentChangeTypes = env->GetMethodID (AndroidAccessibilityEvent,    "setContentChangeTypes", "(I)V");
        }
    }
}

static constexpr auto getClassName (AccessibilityRole role)
{
    switch (role)
    {
        case AccessibilityRole::editableText:  return "android.widget.EditText";
        case AccessibilityRole::toggleButton:  return "android.widget.CheckBox";
        case AccessibilityRole::radioButton:   return "android.widget.RadioButton";
        case AccessibilityRole::image:         return "android.widget.ImageView";
        case AccessibilityRole::popupMenu:     return "android.widget.PopupMenu";
        case AccessibilityRole::comboBox:      return "android.widget.Spinner";
        case AccessibilityRole::tree:          return "android.widget.ExpandableListView";
        case AccessibilityRole::progressBar:   return "android.widget.ProgressBar";

        case AccessibilityRole::scrollBar:
        case AccessibilityRole::slider:        return "android.widget.SeekBar";

        case AccessibilityRole::hyperlink:
        case AccessibilityRole::button:        return "android.widget.Button";

        case AccessibilityRole::label:
        case AccessibilityRole::staticText:    return "android.widget.TextView";

        case AccessibilityRole::tooltip:
        case AccessibilityRole::splashScreen:
        case AccessibilityRole::dialogWindow:  return "android.widget.PopupWindow";

        // If we don't supply a custom class type, then TalkBack will use the node's CollectionInfo
        // to make a sensible decision about how to describe the container
        case AccessibilityRole::list:
        case AccessibilityRole::table:

        case AccessibilityRole::column:
        case AccessibilityRole::row:
        case AccessibilityRole::cell:
        case AccessibilityRole::menuItem:
        case AccessibilityRole::menuBar:
        case AccessibilityRole::listItem:
        case AccessibilityRole::treeItem:
        case AccessibilityRole::window:
        case AccessibilityRole::tableHeader:
        case AccessibilityRole::unspecified:
        case AccessibilityRole::group:
        case AccessibilityRole::ignored:       break;
    }

    return "android.view.View";
}

static jobject getSourceView (const AccessibilityHandler& handler)
{
    if (auto* peer = handler.getComponent().getPeer())
        return (jobject) peer->getNativeHandle();

    return nullptr;
}

//==============================================================================
class AccessibilityNativeHandle
{
public:
    static AccessibilityHandler* getAccessibilityHandlerForVirtualViewId (int virtualViewId)
    {
        auto iter = virtualViewIdMap.find (virtualViewId);

        if (iter != virtualViewIdMap.end())
            return iter->second;

        return nullptr;
    }

    explicit AccessibilityNativeHandle (AccessibilityHandler& h)
        : accessibilityHandler (h),
          virtualViewId (getVirtualViewIdForHandler (accessibilityHandler))
    {
        loadSDKDependentMethods();

        if (virtualViewId != HOST_VIEW_ID)
            virtualViewIdMap[virtualViewId] = &accessibilityHandler;
    }

    ~AccessibilityNativeHandle()
    {
        if (virtualViewId != HOST_VIEW_ID)
            virtualViewIdMap.erase (virtualViewId);
    }

    int getVirtualViewId() const noexcept  { return virtualViewId; }

    void populateNodeInfo (jobject info)
    {
        const ScopedValueSetter<bool> svs (inPopulateNodeInfo, true);

        const auto sourceView = getSourceView (accessibilityHandler);

        if (sourceView == nullptr)
            return;

        auto* env = getEnv();
        auto appContext = getAppContext();

        if (appContext.get() == nullptr)
            return;

        {
            for (auto* child : accessibilityHandler.getChildren())
                env->CallVoidMethod (info, AndroidAccessibilityNodeInfo.addChild,
                                     sourceView, child->getNativeImplementation()->getVirtualViewId());

            if (auto* parent = accessibilityHandler.getParent())
                env->CallVoidMethod (info, AndroidAccessibilityNodeInfo.setVirtualParent,
                                     sourceView, parent->getNativeImplementation()->getVirtualViewId());
            else
                env->CallVoidMethod (info, AndroidAccessibilityNodeInfo.setParent, sourceView);
        }

        {
            const auto scale = Desktop::getInstance().getDisplays().getPrimaryDisplay()->scale;

            const auto screenBounds = accessibilityHandler.getComponent().getScreenBounds() * scale;

            LocalRef<jobject> rect (env->NewObject (AndroidRect, AndroidRect.constructor,
                                                    screenBounds.getX(),     screenBounds.getY(),
                                                    screenBounds.getRight(), screenBounds.getBottom()));

            env->CallVoidMethod (info, AndroidAccessibilityNodeInfo.setBoundsInScreen, rect.get());

            const auto boundsInParent = accessibilityHandler.getComponent().getBoundsInParent() * scale;

            rect = LocalRef<jobject> (env->NewObject (AndroidRect, AndroidRect.constructor,
                                                      boundsInParent.getX(),     boundsInParent.getY(),
                                                      boundsInParent.getRight(), boundsInParent.getBottom()));

            env->CallVoidMethod (info, AndroidAccessibilityNodeInfo.setBoundsInParent, rect.get());
        }

        const auto state = accessibilityHandler.getCurrentState();

        env->CallVoidMethod (info,
                             AndroidAccessibilityNodeInfo.setEnabled,
                             ! state.isIgnored());
        env->CallVoidMethod (info,
                             AndroidAccessibilityNodeInfo.setVisibleToUser,
                             true);
        env->CallVoidMethod (info,
                             AndroidAccessibilityNodeInfo.setPackageName,
                             env->CallObjectMethod (appContext.get(),
                                                    AndroidContext.getPackageName));
        env->CallVoidMethod (info,
                             AndroidAccessibilityNodeInfo.setSource,
                             sourceView,
                             virtualViewId);
        env->CallVoidMethod (info,
                             AndroidAccessibilityNodeInfo.setClassName,
                             javaString (getClassName (accessibilityHandler.getRole())).get());
        env->CallVoidMethod (info,
                             AndroidAccessibilityNodeInfo.setContentDescription,
                             getDescriptionString().get());

        if (state.isFocusable())
        {
            env->CallVoidMethod (info, AndroidAccessibilityNodeInfo.setFocusable, true);

            const auto& component = accessibilityHandler.getComponent();

            if (component.getWantsKeyboardFocus())
            {
                const auto hasKeyboardFocus = component.hasKeyboardFocus (false);

                env->CallVoidMethod (info,
                                     AndroidAccessibilityNodeInfo.setFocused,
                                     hasKeyboardFocus);
                env->CallVoidMethod (info,
                                     AndroidAccessibilityNodeInfo.addAction,
                                     hasKeyboardFocus ? ACTION_CLEAR_FOCUS : ACTION_FOCUS);
            }

            const auto isAccessibleFocused = accessibilityHandler.hasFocus (false);

            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.setAccessibilityFocused,
                                 isAccessibleFocused);

            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.addAction,
                                 isAccessibleFocused ? ACTION_CLEAR_ACCESSIBILITY_FOCUS
                                                     : ACTION_ACCESSIBILITY_FOCUS);
        }

        if (state.isCheckable())
        {
            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.setCheckable,
                                 true);
            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.setChecked,
                                 state.isChecked());
        }

        if (state.isSelectable() || state.isMultiSelectable())
        {
            const auto isSelected = state.isSelected();

            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.setSelected,
                                 isSelected);
            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.addAction,
                                 isSelected ? ACTION_CLEAR_SELECTION : ACTION_SELECT);
        }

        if ((accessibilityHandler.getCurrentState().isCheckable() && accessibilityHandler.getActions().contains (AccessibilityActionType::toggle))
            || accessibilityHandler.getActions().contains (AccessibilityActionType::press))
        {
            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.setClickable,
                                 true);
            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.addAction,
                                 ACTION_CLICK);
        }

        if (accessibilityHandler.getActions().contains (AccessibilityActionType::showMenu)
            && state.isExpandable())
        {
            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.addAction,
                                 state.isExpanded() ? ACTION_COLLAPSE : ACTION_EXPAND);
        }

        if (auto* textInterface = accessibilityHandler.getTextInterface())
        {
            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.setText,
                                 javaString (textInterface->getAllText()).get());

            const auto isReadOnly = textInterface->isReadOnly();

            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.setPassword,
                                 textInterface->isDisplayingProtectedText());

            if (nodeInfoSetEditable != nullptr)
                env->CallVoidMethod (info, nodeInfoSetEditable, ! isReadOnly);

            const auto selection = textInterface->getSelection();

            if (nodeInfoSetTextSelection != nullptr && ! selection.isEmpty())
                env->CallVoidMethod (info,
                                     nodeInfoSetTextSelection,
                                     selection.getStart(), selection.getEnd());

            if (nodeInfoSetLiveRegion != nullptr && accessibilityHandler.hasFocus (false))
                env->CallVoidMethod (info,
                                     nodeInfoSetLiveRegion,
                                     ACCESSIBILITY_LIVE_REGION_POLITE);

            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.setMovementGranularities,
                                 ALL_GRANULARITIES);

            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.addAction,
                                 ACTION_NEXT_AT_MOVEMENT_GRANULARITY);
            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.addAction,
                                 ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY);
            env->CallVoidMethod (info,
                                 AndroidAccessibilityNodeInfo.addAction,
                                 ACTION_SET_SELECTION);

            if (! isReadOnly)
                env->CallVoidMethod (info, AndroidAccessibilityNodeInfo.addAction, ACTION_SET_TEXT);
        }

        if (auto* valueInterface = accessibilityHandler.getValueInterface())
        {
            if (! valueInterface->isReadOnly())
            {
                const auto range = valueInterface->getRange();

                if (range.isValid())
                {
                    env->CallVoidMethod (info,
                                         AndroidAccessibilityNodeInfo.addAction,
                                         ACTION_SCROLL_FORWARD);
                    env->CallVoidMethod (info,
                                         AndroidAccessibilityNodeInfo.addAction,
                                         ACTION_SCROLL_BACKWARD);
                }
            }
        }

        if (getAndroidSDKVersion() >= 19)
        {
            if (auto* tableInterface = accessibilityHandler.getTableInterface())
            {
                const auto rows    = tableInterface->getNumRows();
                const auto columns = tableInterface->getNumColumns();
                const LocalRef<jobject> collectionInfo { env->CallStaticObjectMethod (AndroidAccessibilityNodeInfoCollectionInfo,
                                                                                      AndroidAccessibilityNodeInfoCollectionInfo.obtain,
                                                                                      (jint) rows,
                                                                                      (jint) columns,
                                                                                      (jboolean) false) };
                env->CallVoidMethod (info, AndroidAccessibilityNodeInfo19.setCollectionInfo, collectionInfo.get());
            }

            if (auto* enclosingTableHandler = getEnclosingHandlerWithInterface (&accessibilityHandler, &AccessibilityHandler::getTableInterface))
            {
                auto* interface = enclosingTableHandler->getTableInterface();
                jassert (interface != nullptr);
                const auto rowSpan    = interface->getRowSpan    (accessibilityHandler);
                const auto columnSpan = interface->getColumnSpan (accessibilityHandler);

                enum class IsHeader { no, yes };

                const auto addCellInfo = [env, &info] (AccessibilityTableInterface::Span rows, AccessibilityTableInterface::Span columns, IsHeader header)
                {
                    const LocalRef<jobject> collectionItemInfo { env->CallStaticObjectMethod (AndroidAccessibilityNodeInfoCollectionItemInfo,
                                                                                              AndroidAccessibilityNodeInfoCollectionItemInfo.obtain,
                                                                                              (jint) rows.begin,
                                                                                              (jint) rows.num,
                                                                                              (jint) columns.begin,
                                                                                              (jint) columns.num,
                                                                                              (jboolean) (header == IsHeader::yes)) };
                    env->CallVoidMethod (info, AndroidAccessibilityNodeInfo19.setCollectionItemInfo, collectionItemInfo.get());
                };

                if (rowSpan.hasValue() && columnSpan.hasValue())
                {
                    addCellInfo (*rowSpan, *columnSpan, IsHeader::no);
                }
                else
                {
                    if (auto* tableHeader = interface->getHeaderHandler())
                    {
                        if (accessibilityHandler.getParent() == tableHeader)
                        {
                            const auto children = tableHeader->getChildren();
                            const auto column = std::distance (children.cbegin(), std::find (children.cbegin(), children.cend(), &accessibilityHandler));

                            // Talkback will only treat a row as a column header if its row index is zero
                            // https://github.com/google/talkback/blob/acd0bc7631a3dfbcf183789c7557596a45319e1f/utils/src/main/java/CollectionState.java#L853
                            addCellInfo ({ 0, 1 }, { (int) column, 1 }, IsHeader::yes);
                        }
                    }
                }
            }
        }
    }

    bool performAction (int action, jobject arguments)
    {
        switch (action)
        {
            case ACTION_ACCESSIBILITY_FOCUS:
            {
                const WeakReference<Component> safeComponent (&accessibilityHandler.getComponent());

                accessibilityHandler.getActions().invoke (AccessibilityActionType::focus);

                if (safeComponent != nullptr)
                    accessibilityHandler.grabFocus();

                return true;
            }

            case ACTION_CLEAR_ACCESSIBILITY_FOCUS:
            {
                accessibilityHandler.giveAwayFocus();
                return true;
            }

            case ACTION_FOCUS:
            case ACTION_CLEAR_FOCUS:
            {
                auto& component = accessibilityHandler.getComponent();

                if (component.getWantsKeyboardFocus())
                {
                    const auto hasFocus = component.hasKeyboardFocus (false);

                    if (hasFocus && action == ACTION_CLEAR_FOCUS)
                        component.giveAwayKeyboardFocus();
                    else if (! hasFocus && action == ACTION_FOCUS)
                        component.grabKeyboardFocus();

                    return true;
                }

                break;
            }

            case ACTION_CLICK:
            {
                // Invoking the action may delete this handler
                const WeakReference<AccessibilityNativeHandle> savedHandle { this };

                if ((accessibilityHandler.getCurrentState().isCheckable() && accessibilityHandler.getActions().invoke (AccessibilityActionType::toggle))
                    || accessibilityHandler.getActions().invoke (AccessibilityActionType::press))
                {
                    if (savedHandle != nullptr)
                        sendAccessibilityEventImpl (accessibilityHandler, TYPE_VIEW_CLICKED, 0);

                    return true;
                }

                break;
            }

            case ACTION_SELECT:
            case ACTION_CLEAR_SELECTION:
            {
                const auto state = accessibilityHandler.getCurrentState();

                if (state.isSelectable() || state.isMultiSelectable())
                {
                    const auto isSelected = state.isSelected();

                    if ((isSelected && action == ACTION_CLEAR_SELECTION)
                        || (! isSelected && action == ACTION_SELECT))
                    {
                        return accessibilityHandler.getActions().invoke (AccessibilityActionType::toggle);
                    }

                }

                break;
            }

            case ACTION_EXPAND:
            case ACTION_COLLAPSE:
            {
                const auto state = accessibilityHandler.getCurrentState();

                if (state.isExpandable())
                {
                    const auto isExpanded = state.isExpanded();

                    if ((isExpanded && action == ACTION_COLLAPSE)
                        || (! isExpanded && action == ACTION_EXPAND))
                    {
                        return accessibilityHandler.getActions().invoke (AccessibilityActionType::showMenu);
                    }
                }

                break;
            }

            case ACTION_NEXT_AT_MOVEMENT_GRANULARITY:      return moveCursor (arguments, true);
            case ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY:  return moveCursor (arguments, false);

            case ACTION_SET_SELECTION:
            {
                if (auto* textInterface = accessibilityHandler.getTextInterface())
                {
                    auto* env = getEnv();

                    const auto selection = [&]() -> Range<int>
                    {
                        const auto selectionStartKey = javaString ("ACTION_ARGUMENT_SELECTION_START_INT");
                        const auto selectionEndKey   = javaString ("ACTION_ARGUMENT_SELECTION_END_INT");

                        const auto hasKey = [&env, &arguments] (const auto& key)
                        {
                            return env->CallBooleanMethod (arguments, AndroidBundle.containsKey, key.get());
                        };

                        if (hasKey (selectionStartKey) && hasKey (selectionEndKey))
                        {
                            const auto getKey = [&env, &arguments] (const auto& key)
                            {
                                return env->CallIntMethod (arguments, AndroidBundle.getInt, key.get());
                            };

                            const auto start = getKey (selectionStartKey);
                            const auto end   = getKey (selectionEndKey);

                            return Range<int>::between (start, end);
                        }

                        return {};
                    }();

                    textInterface->setSelection (selection);

                    return true;
                }

                break;
            }

            case ACTION_SET_TEXT:
            {
                if (auto* textInterface = accessibilityHandler.getTextInterface())
                {
                    if (! textInterface->isReadOnly())
                    {
                        const auto charSequenceKey = javaString ("ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE");

                        auto* env = getEnv();

                        const auto text = [&]() -> String
                        {
                            if (env->CallBooleanMethod (arguments, AndroidBundle.containsKey, charSequenceKey.get()))
                            {
                                LocalRef<jobject> charSequence (env->CallObjectMethod (arguments,
                                                                                       AndroidBundle.getCharSequence,
                                                                                       charSequenceKey.get()));
                                LocalRef<jstring> textStringRef ((jstring) env->CallObjectMethod (charSequence,
                                                                                                  JavaCharSequence.toString));

                                return juceString (textStringRef.get());
                            }

                            return {};
                        }();

                        textInterface->setText (text);
                    }
                }

                break;
            }

            case ACTION_SCROLL_BACKWARD:
            case ACTION_SCROLL_FORWARD:
            {
                if (auto* valueInterface = accessibilityHandler.getValueInterface())
                {
                    if (! valueInterface->isReadOnly())
                    {
                        const auto range = valueInterface->getRange();

                        if (range.isValid())
                        {
                            const auto interval = action == ACTION_SCROLL_BACKWARD ? -range.getInterval()
                                                                                   : range.getInterval();
                            valueInterface->setValue (jlimit (range.getMinimumValue(),
                                                              range.getMaximumValue(),
                                                              valueInterface->getCurrentValue() + interval));

                            // required for Android to announce the new value
                            sendAccessibilityEventImpl (accessibilityHandler, TYPE_VIEW_SELECTED, 0);
                            return true;
                        }
                    }
                }

                break;
            }
        }

        return false;
    }

    bool isInPopulateNodeInfo() const noexcept  { return inPopulateNodeInfo; }

    static bool areAnyAccessibilityClientsActive()
    {
        auto* env = getEnv();
        auto appContext = getAppContext();

        if (appContext.get() != nullptr)
        {
            LocalRef<jobject> accessibilityManager (env->CallObjectMethod (appContext.get(), AndroidContext.getSystemService,
                                                                           javaString ("accessibility").get()));

            if (accessibilityManager != nullptr)
                return env->CallBooleanMethod (accessibilityManager.get(), AndroidAccessibilityManager.isEnabled);
        }

        return false;
    }

    template <typename ModificationCallback>
    static void sendAccessibilityEventExtendedImpl (const AccessibilityHandler& handler,
                                                    int eventType,
                                                    ModificationCallback&& modificationCallback)
    {
        if (! areAnyAccessibilityClientsActive())
            return;

        if (const auto sourceView = getSourceView (handler))
        {
            const auto* nativeImpl = handler.getNativeImplementation();

            if (nativeImpl == nullptr || nativeImpl->isInPopulateNodeInfo())
                return;

            auto* env = getEnv();
            auto appContext = getAppContext();

            if (appContext.get() == nullptr)
                return;

            LocalRef<jobject> event (env->CallStaticObjectMethod (AndroidAccessibilityEvent,
                                                                  AndroidAccessibilityEvent.obtain,
                                                                  eventType));

            env->CallVoidMethod (event,
                                 AndroidAccessibilityEvent.setPackageName,
                                 env->CallObjectMethod (appContext.get(),
                                                        AndroidContext.getPackageName));

            env->CallVoidMethod (event,
                                 AndroidAccessibilityEvent.setSource,
                                 sourceView,
                                 nativeImpl->getVirtualViewId());

            modificationCallback (event);

            env->CallBooleanMethod (sourceView,
                                    AndroidViewGroup.requestSendAccessibilityEvent,
                                    sourceView,
                                    event.get());
        }
    }

    static void sendAccessibilityEventImpl (const AccessibilityHandler& handler, int eventType, int contentChangeTypes)
    {
        sendAccessibilityEventExtendedImpl (handler, eventType, [contentChangeTypes] (auto event)
        {
            if (contentChangeTypes != 0 && accessibilityEventSetContentChangeTypes != nullptr)
                getEnv()->CallVoidMethod (event,
                                          accessibilityEventSetContentChangeTypes,
                                          contentChangeTypes);
        });
    }

private:
    static std::unordered_map<int, AccessibilityHandler*> virtualViewIdMap;

    static int getVirtualViewIdForHandler (const AccessibilityHandler& handler)
    {
        static int counter = 0;

        if (handler.getComponent().isOnDesktop())
            return HOST_VIEW_ID;

        return counter++;
    }

    LocalRef<jstring> getDescriptionString() const
    {
        const auto valueString = [this]() -> String
        {
            if (auto* textInterface = accessibilityHandler.getTextInterface())
                return textInterface->getAllText();

            if (auto* valueInterface = accessibilityHandler.getValueInterface())
                return valueInterface->getCurrentValueAsString();

            return {};
        }();

        StringArray strings (accessibilityHandler.getTitle(),
                             valueString,
                             accessibilityHandler.getDescription(),
                             accessibilityHandler.getHelp());

        strings.removeEmptyStrings();

        return javaString (strings.joinIntoString (","));
    }

    bool moveCursor (jobject arguments, bool forwards)
    {
        using ATH = AccessibilityTextHelpers;

        auto* textInterface = accessibilityHandler.getTextInterface();

        if (textInterface == nullptr)
            return false;

        const auto granularityKey = javaString ("ACTION_ARGUMENT_MOVEMENT_GRANULARITY_INT");
        const auto extendSelectionKey = javaString ("ACTION_ARGUMENT_EXTEND_SELECTION_BOOLEAN");

        auto* env = getEnv();

        const auto boundaryType = [&]
        {
            const auto granularity = env->CallIntMethod (arguments, AndroidBundle.getInt, granularityKey.get());

            using BoundaryType = ATH::BoundaryType;

            switch (granularity)
            {
                case MOVEMENT_GRANULARITY_CHARACTER:  return BoundaryType::character;
                case MOVEMENT_GRANULARITY_WORD:       return BoundaryType::word;
                case MOVEMENT_GRANULARITY_LINE:       return BoundaryType::line;
                case MOVEMENT_GRANULARITY_PARAGRAPH:
                case MOVEMENT_GRANULARITY_PAGE:       return BoundaryType::document;
            }

            jassertfalse;
            return BoundaryType::character;
        }();

        const auto direction = forwards
                             ? ATH::Direction::forwards
                             : ATH::Direction::backwards;

        const auto extend = env->CallBooleanMethod (arguments, AndroidBundle.getBoolean, extendSelectionKey.get())
                          ? ATH::ExtendSelection::yes
                          : ATH::ExtendSelection::no;

        const auto oldSelection = textInterface->getSelection();
        const auto newSelection = ATH::findNewSelectionRangeAndroid (*textInterface, boundaryType, extend, direction);
        textInterface->setSelection (newSelection);

        // Required for Android to read back the text that the cursor moved over
        sendAccessibilityEventExtendedImpl (accessibilityHandler, TYPE_VIEW_TEXT_TRAVERSED_AT_MOVEMENT_GRANULARITY, [&] (auto event)
        {
            env->CallVoidMethod (event,
                                 AndroidAccessibilityEvent.setAction,
                                 forwards ? ACTION_NEXT_AT_MOVEMENT_GRANULARITY : ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY);

            env->CallVoidMethod (event,
                                 AndroidAccessibilityEvent.setFromIndex,
                                 oldSelection.getStart() != newSelection.getStart() ? oldSelection.getStart()
                                                                                    : oldSelection.getEnd());

            env->CallVoidMethod (event,
                                 AndroidAccessibilityEvent.setToIndex,
                                 oldSelection.getStart() != newSelection.getStart() ? newSelection.getStart()
                                                                                    : newSelection.getEnd());
        });

        return true;
    }

    AccessibilityHandler& accessibilityHandler;
    const int virtualViewId;
    bool inPopulateNodeInfo = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityNativeHandle)

    JUCE_DECLARE_WEAK_REFERENCEABLE (AccessibilityNativeHandle)
};

std::unordered_map<int, AccessibilityHandler*> AccessibilityNativeHandle::virtualViewIdMap;

class AccessibilityHandler::AccessibilityNativeImpl : public AccessibilityNativeHandle
{
public:
    using AccessibilityNativeHandle::AccessibilityNativeHandle;
};

//==============================================================================
AccessibilityNativeHandle* AccessibilityHandler::getNativeImplementation() const
{
    return nativeImpl.get();
}

void notifyAccessibilityEventInternal (const AccessibilityHandler& handler,
                                       InternalAccessibilityEvent eventType)
{
    if (eventType == InternalAccessibilityEvent::elementCreated
        || eventType == InternalAccessibilityEvent::elementDestroyed
        || eventType == InternalAccessibilityEvent::elementMovedOrResized)
    {
        if (auto* parent = handler.getParent())
            AccessibilityNativeHandle::sendAccessibilityEventImpl (*parent, TYPE_WINDOW_CONTENT_CHANGED, CONTENT_CHANGE_TYPE_SUBTREE);

        return;
    }

    auto notification = [&handler, eventType]
    {
        switch (eventType)
        {
            case InternalAccessibilityEvent::focusChanged:
                return handler.hasFocus (false) ? TYPE_VIEW_ACCESSIBILITY_FOCUSED
                                                : TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED;

            case InternalAccessibilityEvent::elementCreated:
            case InternalAccessibilityEvent::elementDestroyed:
            case InternalAccessibilityEvent::elementMovedOrResized:
            case InternalAccessibilityEvent::windowOpened:
            case InternalAccessibilityEvent::windowClosed:
                break;
        }

        return 0;
    }();

    if (notification != 0)
        AccessibilityNativeHandle::sendAccessibilityEventImpl (handler, notification, 0);
}

void AccessibilityHandler::notifyAccessibilityEvent (AccessibilityEvent eventType) const
{
    auto notification = [eventType]
    {
        switch (eventType)
        {
            case AccessibilityEvent::textSelectionChanged:  return TYPE_VIEW_TEXT_SELECTION_CHANGED;
            case AccessibilityEvent::textChanged:           return TYPE_VIEW_TEXT_CHANGED;

            case AccessibilityEvent::titleChanged:
            case AccessibilityEvent::structureChanged:      return TYPE_WINDOW_CONTENT_CHANGED;

            case AccessibilityEvent::rowSelectionChanged:
            case AccessibilityEvent::valueChanged:          break;
        }

        return 0;
    }();

    if (notification == 0)
        return;

    const auto contentChangeTypes = [eventType]
    {
        if (eventType == AccessibilityEvent::titleChanged)      return CONTENT_CHANGE_TYPE_CONTENT_DESCRIPTION;
        if (eventType == AccessibilityEvent::structureChanged)  return CONTENT_CHANGE_TYPE_SUBTREE;

        return 0;
    }();

    AccessibilityNativeHandle::sendAccessibilityEventImpl (*this, notification, contentChangeTypes);
}

void AccessibilityHandler::postAnnouncement (const String& announcementString,
                                             AnnouncementPriority)
{
    if (! AccessibilityNativeHandle::areAnyAccessibilityClientsActive())
        return;

    const auto rootView = []
    {
        LocalRef<jobject> activity (getMainActivity());

        if (activity != nullptr)
        {
            auto* env = getEnv();

            LocalRef<jobject> mainWindow (env->CallObjectMethod (activity.get(), AndroidActivity.getWindow));
            LocalRef<jobject> decorView (env->CallObjectMethod (mainWindow.get(), AndroidWindow.getDecorView));

            return LocalRef<jobject> (env->CallObjectMethod (decorView.get(), AndroidView.getRootView));
        }

        return LocalRef<jobject>();
    }();

    if (rootView != nullptr)
        getEnv()->CallVoidMethod (rootView.get(),
                                  AndroidView.announceForAccessibility,
                                  javaString (announcementString).get());
}

} // namespace juce
