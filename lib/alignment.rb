#!/usr/bin/env ruby
#
# alignment.rb - Alignment functions
#
# Written by Marius L. Jøhndal <mariuslj at ifi.uio.no>, 2008
#
require 'gale_church'

module Alignment
  def self.is_alignable?(o)
    o.responds_to?(:weight)
  end

  # Represents an alignment of two regions, one +left+ region and one
  # +right+ region. Both regions are arrays of alignable objects.
  AlignedRegions = Struct.new(:left, :right)

  class AlignedRegions
    def to_s
      "<#{self.left},#{self.right}>"
    end
  end

  public

  # Aligns two arrays of regions +a+ and +b+. Returns an array of
  # +AlignedRegions+.
  def self.align_regions(a, b, method = :gale_church)
    case method
    when :gale_church
      x, y = Alignment::GaleChurch::align(a.collect(&:weight), b.collect(&:weight))
      raise "Error aligning regions: returned block count does not match" unless x.length == y.length

      x.zip(y).collect { |r, s| AlignedRegions.new(r.collect { |i| a[i] }, s.collect { |i| b[i] }) }
    else
      raise ArgumentError.new("invalid method #{method}")
    end
  end

  class AlignableText
    attr_reader :text

    def initialize(text)
      @text = text
    end

    def weight
      # FIXME: character length
      @text.length
      # FIXME: word count
      @text.split(' ').length
    end

    def to_s
      @text
    end
  end

  ANCHOR_REGEXP = /\s*\|\|\s*/
  BOUNDARY_REGEXP = /\s*\|\s*/

  # Aligns to strings of text +a+ and +b+ using method +method+. The boundaries of the
  # blocks to be aligned are denoted by a | character (with or without surrounding
  # white-space). The sequence || denotes an anchor (or hard delimiter), i.e. a fixed 
  # synchronisation point. There must be the same number of anchors in both strings,
  # but the number of boundaries may be different. The returned value is an array
  # of pairs of aligned strings.
  def self.align_text(a, b, method = :gale_church)
    result = []

    regions_a = a.split(ANCHOR_REGEXP)
    regions_b = b.split(ANCHOR_REGEXP)

    raise ArgumentError.new("different number of anchors in strings") unless regions_a.length == regions_b.length

    regions_a.zip(regions_b).each do |x, y|
      r = align_regions(x.split(BOUNDARY_REGEXP).collect { |region| AlignableText.new(region) },
                        y.split(BOUNDARY_REGEXP).collect { |region| AlignableText.new(region) },
                       method)
      r.collect! { |i| [ i.left.collect(&:text).join(' '), i.right.collect(&:text).join(' ') ] }
      result += r
    end

    result
  end
end
