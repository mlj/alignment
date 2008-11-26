require 'alignment'
require 'test/unit'

class AlignmentTestCase < Test::Unit::TestCase
  def test_church_alignment
    x = [ Alignment::AlignableText.new("foo") ]
    y = [ Alignment::AlignableText.new("bar") ]
    assert_equal "<foo,bar>", Alignment::align_regions(x, y).collect(&:to_s).join(",")

    x = [ Alignment::AlignableText.new("foo") ]
    y = [ Alignment::AlignableText.new("bar"), Alignment::AlignableText.new("baz") ]
    assert_equal "<foo,barbaz>", Alignment::align_regions(x, y).collect(&:to_s).join(",")

    x = [ ]
    y = [ Alignment::AlignableText.new("bar") ]
    assert_equal "<,bar>", Alignment::align_regions(x, y).collect(&:to_s).join(",")

    x = [ ]
    y = [ ]
    assert_equal "", Alignment::align_regions(x, y).collect(&:to_s).join(",")

    x = [ Alignment::AlignableText.new("foo"), Alignment::AlignableText.new("koo") ]
    y = [ Alignment::AlignableText.new("bar"), Alignment::AlignableText.new("baz") ]
    assert_equal "<foo,bar>,<koo,baz>", Alignment::align_regions(x, y).collect(&:to_s).join(",")
  end

  def test_church_text_alignment
    assert_equal [["The quick brown fox", "Den kvikke brune reven"], ["jumps", "hopper"], ["over the lazy dog", "over den trege hunden"]],
      Alignment::align_text("The quick brown fox | jumps | over the lazy dog",
                            "Den kvikke brune reven | hopper | over den trege hunden")

    assert_equal [["The quick brown fox", "Den kvikke brune reven"], ["jumps", "hopper elegant"], ["over the lazy dog", "over den trege hunden"]],
      Alignment::align_text("The quick brown fox | jumps | over the lazy dog",
                            "Den kvikke brune reven | hopper elegant | over den trege hunden")

    assert_equal [["The quick brown fox", "Den kvikke brune reven"], ["jumps", "hopper"], ["over the lazy dog", "elegant over den trege hunden"]],
      Alignment::align_text("The quick brown fox | jumps || over the lazy dog",
                            "Den kvikke brune reven | hopper || elegant | over den trege hunden")
  end
end
    
