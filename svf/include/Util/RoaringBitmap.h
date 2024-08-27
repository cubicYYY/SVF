#ifndef ROARING_BITMAP_H_
#define ROARING_BITMAP_H_

#ifndef ROARING_EXCEPTIONS
#    define ROARING_EXCEPTIONS 0
#endif

#include <cstdint>
#include "roaring.hh" // the amalgamated roaring.hh includes roaring64map.hh

namespace SVF
{
/// Roaring Bitmap Wrapper
/// Its iterator has built-in "current element" optimization.
class RoaringBitmap
{
    friend class SVFIRWriter;
    friend class SVFIRReader;

public:
    class RoaringBitmapIterator;
    using iterator = RoaringBitmapIterator;
    using const_iterator = const RoaringBitmapIterator;
    using size_type = uint32_t;

public:
    RoaringBitmap() : roaring({})
    {
        roaring.setCopyOnWrite(true);
    }
    RoaringBitmap(const RoaringBitmap& RHS) : roaring(RHS.roaring)
    {
        roaring.setCopyOnWrite(true);
    }
    RoaringBitmap(RoaringBitmap&& RHS) noexcept : roaring(std::move(RHS.roaring))
    {
        roaring.setCopyOnWrite(true);
    }
    RoaringBitmap(const roaring::Roaring& r) noexcept : roaring(r)
    {
        roaring.setCopyOnWrite(true);
    }
    RoaringBitmap(roaring::Roaring&& r) noexcept : roaring(std::move(r))
    {
        roaring.setCopyOnWrite(true);
    }

    /// Wrapper class of CRoaring Bitmap
    class RoaringBitmapIterator
    {
        friend class SVFIRWriter;
        friend class SVFIRReader;

    public:
        RoaringBitmapIterator(void) = delete;
        RoaringBitmapIterator(const RoaringBitmap* rbm, bool end = false);

        RoaringBitmapIterator(const RoaringBitmapIterator& rbmIt) = default;
        RoaringBitmapIterator(RoaringBitmapIterator&& rbmIt) = default;
        RoaringBitmapIterator& operator=(const RoaringBitmapIterator& rbmIt) = default;
        RoaringBitmapIterator& operator=(RoaringBitmapIterator&& rbmIt) = default;

        /// `++it`
        const_iterator& operator++(void);

        /// `it++`
        const_iterator operator++(int);

        /// `*it`
        uint32_t operator*(void) const;

        bool operator==(const_iterator& rhs) const;
        bool operator!=(const_iterator& rhs) const;

    private:
        roaring::RoaringSetBitForwardIterator it_;
    };

    const_iterator begin(void) const;
    const_iterator end(void) const;
    bool empty(void) const noexcept;
    bool test(size_type idx) const noexcept;
    void set(size_type idx) noexcept;
    void reset(size_type idx) noexcept;
    bool test_and_set(size_type idx) noexcept;
    bool contains(const RoaringBitmap& RHS) const noexcept;
    bool intersects(const RoaringBitmap& RHS) const noexcept;
    size_type count(void) const noexcept;
    void clear(void) noexcept;

    bool operator==(const RoaringBitmap& rhs) const noexcept;
    bool operator!=(const RoaringBitmap& rhs) const noexcept;
    bool operator|=(const RoaringBitmap& rhs) noexcept;
    bool operator&=(const RoaringBitmap& rhs) noexcept;
    RoaringBitmap& operator=(const RoaringBitmap& rhs) noexcept;
    RoaringBitmap& operator=(RoaringBitmap&& rhs) noexcept;
    RoaringBitmap operator&(const RoaringBitmap& rhs) const noexcept;
    RoaringBitmap operator|(const RoaringBitmap& rhs) const noexcept;
    RoaringBitmap operator-(const RoaringBitmap& rhs) const noexcept;
    bool intersectWithComplement(const RoaringBitmap& rhs) noexcept;
    void intersectWithComplement(const RoaringBitmap& lhs, const RoaringBitmap& rhs) noexcept;
    size_t hash(void) const;
    void invalidateBulk() noexcept;
    static void logOperation(const std::string& operation, const RoaringBitmap& lhs, const RoaringBitmap& rhs);
    static void logSingleOperation(const std::string& operation, const RoaringBitmap& rhs, const int idx=-1);
public:
    roaring::Roaring roaring;
    mutable roaring::BulkContext bulk{};
};

} // namespace SVF

#endif // ROARING_BITMAP_H_