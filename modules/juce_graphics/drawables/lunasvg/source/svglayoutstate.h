#ifndef LUNASVG_SVGLAYOUTSTATE_H
#define LUNASVG_SVGLAYOUTSTATE_H

#include "svgproperty.h"
namespace {

namespace lunasvg {

class SVGLayoutState {
public:
    SVGLayoutState() = default;
    SVGLayoutState(const SVGLayoutState& parent, const SVGElement* element);

    const SVGLayoutState* parent() const { return m_parent; }
    const SVGElement* element() const { return m_element; }

    const Paint& fill() const { return m_fill; }
    const Paint& stroke() const { return m_stroke; }

    const Color& color() const { return m_color; }
    const Color& stop_color() const { return m_stop_color; }

    float opacity() const { return m_opacity; }
    float stop_opacity() const { return m_stop_opacity; }
    float fill_opacity() const { return m_fill_opacity; }
    float stroke_opacity() const { return m_stroke_opacity; }
    float stroke_miterlimit() const { return m_stroke_miterlimit; }
    float font_size() const { return m_font_size; }

    const Length& letter_spacing() const { return m_letter_spacing; }
    const Length& word_spacing() const { return m_word_spacing; }

    const BaselineShift& baseline_shit() const { return m_baseline_shit; }
    const Length& stroke_width() const { return m_stroke_width; }
    const Length& stroke_dashoffset() const { return m_stroke_dashoffset; }
    const LengthList& stroke_dasharray() const { return m_stroke_dasharray; }

    LineCap stroke_linecap() const { return m_stroke_linecap; }
    LineJoin stroke_linejoin() const { return m_stroke_linejoin; }

    FillRule fill_rule() const { return m_fill_rule; }
    FillRule clip_rule() const { return m_clip_rule; }

    FontWeight font_weight() const { return m_font_weight; }
    FontStyle font_style() const { return m_font_style; }

    AlignmentBaseline alignment_baseline() const { return m_alignment_baseline; }
    DominantBaseline dominant_baseline() const { return m_dominant_baseline; }

    TextAnchor text_anchor() const { return m_text_anchor; }
    WhiteSpace white_space() const { return m_white_space; }
    WritingMode writing_mode() const { return m_writing_mode; }
    TextOrientation text_orientation() const { return m_text_orientation; }
    Direction direction() const { return m_direction; }

    Display display() const { return m_display; }
    Visibility visibility() const { return m_visibility; }
    Overflow overflow() const { return m_overflow; }
    PointerEvents pointer_events() const { return m_pointer_events; }
    MaskType mask_type() const { return m_mask_type; }

    const std::string& mask() const { return m_mask; }
    const std::string& clip_path() const { return m_clip_path; }
    const std::string& marker_start() const { return m_marker_start; }
    const std::string& marker_mid() const { return m_marker_mid; }
    const std::string& marker_end() const { return m_marker_end; }
    const std::string& font_family() const { return m_font_family; }

    Font font() const;

private:
    const SVGLayoutState* m_parent = nullptr;
    const SVGElement* m_element = nullptr;

    Paint m_fill{Color::Black};
    Paint m_stroke{Color::Transparent};

    Color m_color = Color::Black;
    Color m_stop_color = Color::Black;

    float m_opacity = 1.f;
    float m_fill_opacity = 1.f;
    float m_stroke_opacity = 1.f;
    float m_stop_opacity = 1.f;
    float m_stroke_miterlimit = 4.f;
    float m_font_size = 12.f;

    Length m_letter_spacing{0.f, LengthUnits::None};
    Length m_word_spacing{0.f, LengthUnits::None};

    BaselineShift m_baseline_shit;
    Length m_stroke_width{1.f, LengthUnits::None};
    Length m_stroke_dashoffset{0.f, LengthUnits::None};
    LengthList m_stroke_dasharray;

    LineCap m_stroke_linecap = LineCap::Butt;
    LineJoin m_stroke_linejoin = LineJoin::Miter;

    FillRule m_fill_rule = FillRule::NonZero;
    FillRule m_clip_rule = FillRule::NonZero;

    FontWeight m_font_weight = FontWeight::Normal;
    FontStyle m_font_style = FontStyle::Normal;

    AlignmentBaseline m_alignment_baseline = AlignmentBaseline::Auto;
    DominantBaseline m_dominant_baseline = DominantBaseline::Auto;

    TextAnchor m_text_anchor = TextAnchor::Start;
    WhiteSpace m_white_space = WhiteSpace::Default;
    WritingMode m_writing_mode = WritingMode::Horizontal;
    TextOrientation m_text_orientation = TextOrientation::Mixed;
    Direction m_direction = Direction::Ltr;

    Display m_display = Display::Inline;
    Visibility m_visibility = Visibility::Visible;
    Overflow m_overflow = Overflow::Visible;
    PointerEvents m_pointer_events = PointerEvents::Auto;
    MaskType m_mask_type = MaskType::Luminance;

    std::string m_mask;
    std::string m_clip_path;
    std::string m_marker_start;
    std::string m_marker_mid;
    std::string m_marker_end;
    std::string m_font_family;
};

} // namespace lunasvg
}

#endif // LUNASVG_SVGLAYOUTSTATE_H
