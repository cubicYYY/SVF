#include "Util/RoaringBitmap.h"
#include "roaring.c"

namespace SVF
{
RoaringBitmap::RoaringBitmapIterator::RoaringBitmapIterator(const RoaringBitmap* rbm, bool end)
    : it_(rbm->roaring, end){};

RoaringBitmap::const_iterator& RoaringBitmap::RoaringBitmapIterator::operator++(void)
{
    ++it_;
    return *this;
}

RoaringBitmap::const_iterator RoaringBitmap::RoaringBitmapIterator::operator++(int)
{
    RoaringBitmap::const_iterator old = *this;
    ++(*this);
    return old;
}

uint32_t RoaringBitmap::RoaringBitmapIterator::operator*(void) const
{
    return *it_;
}

bool RoaringBitmap::RoaringBitmapIterator::operator==(RoaringBitmap::const_iterator& rhs) const
{
    return rhs.it_ == it_;
}
bool RoaringBitmap::RoaringBitmapIterator::operator!=(RoaringBitmap::const_iterator& rhs) const
{
    return rhs.it_ != it_;
}

RoaringBitmap::const_iterator RoaringBitmap::end(void) const
{
    return RoaringBitmapIterator(this, true);
}

RoaringBitmap::const_iterator RoaringBitmap::begin(void) const
{
    return RoaringBitmapIterator(this);
}

bool RoaringBitmap::empty(void) const noexcept
{
    return roaring.isEmpty();
}

bool RoaringBitmap::test(size_type idx) const noexcept
{
    return roaring.containsBulk(bulk, idx);
}

void RoaringBitmap::set(size_type idx) noexcept
{
    return roaring.addBulk(bulk, idx);
}

void RoaringBitmap::reset(size_type idx) noexcept
{
    invalidateBulk();
    return roaring.remove(idx);
}

bool RoaringBitmap::test_and_set(size_type idx) noexcept
{
    invalidateBulk();
    return roaring.addChecked(idx);
}

bool RoaringBitmap::contains(const RoaringBitmap& RHS) const noexcept
{
    return RHS.roaring.isSubset(roaring);
}

bool RoaringBitmap::intersects(const RoaringBitmap& RHS) const noexcept
{
    return RHS.roaring.intersect(roaring);
}

RoaringBitmap::size_type RoaringBitmap::count(void) const noexcept
{
    return (size_type)roaring.cardinality(); // FIXME: safe?
}

void RoaringBitmap::clear(void) noexcept
{
    invalidateBulk();
    roaring.clear();
}

bool RoaringBitmap::operator==(const RoaringBitmap& rhs) const noexcept
{
    return roaring == rhs.roaring;
}

bool RoaringBitmap::operator!=(const RoaringBitmap& rhs) const noexcept
{
    return !(roaring == rhs.roaring);
}

bool RoaringBitmap::operator|=(const RoaringBitmap& rhs) noexcept
{
    // TODO: Avoid cardinality counting
    invalidateBulk();
    auto card = roaring.cardinality();
    roaring |= rhs.roaring;
    return card != roaring.cardinality();
}

bool RoaringBitmap::operator&=(const RoaringBitmap& rhs) noexcept
{
    // TODO: Avoid cardinality counting
    invalidateBulk();
    auto card = roaring.cardinality();
    roaring &= rhs.roaring;
    return card != roaring.cardinality();
}

bool RoaringBitmap::intersectWithComplement(const RoaringBitmap& rhs) noexcept
{
    // TODO: Avoid cardinality counting
    invalidateBulk();
    auto card = roaring.cardinality();
    roaring -= rhs.roaring;
    return card != roaring.cardinality();
}

void RoaringBitmap::intersectWithComplement(const RoaringBitmap& lhs, const RoaringBitmap& rhs) noexcept
{
    // TODO: faster?
    invalidateBulk();
    roaring = lhs.roaring;
    roaring -= rhs.roaring;
}

size_t RoaringBitmap::hash(void) const
{
    // TODO: Use a real hash!
    auto first = roaring.begin();
    auto last = --roaring.end();
    auto first_val = first.i.has_value ? *first : 0;
    auto last_val = last.i.has_value ? *last : 0;

    return roaring.cardinality() * 961 + first_val * 31 + last_val;
}

RoaringBitmap& RoaringBitmap::operator=(const RoaringBitmap& rhs) noexcept
{
    invalidateBulk();
    roaring = rhs.roaring;
    return *this;
}
RoaringBitmap& RoaringBitmap::operator=(RoaringBitmap&& rhs) noexcept
{
    invalidateBulk();
    roaring = std::move(rhs.roaring);
    return *this;
}

RoaringBitmap RoaringBitmap::operator&(const RoaringBitmap& rhs) const noexcept
{
    RoaringBitmap result(roaring & rhs.roaring);
    return result;
}

RoaringBitmap RoaringBitmap::operator|(const RoaringBitmap& rhs) const noexcept
{
    RoaringBitmap result(roaring | rhs.roaring);
    return result;
}

RoaringBitmap RoaringBitmap::operator-(const RoaringBitmap& rhs) const noexcept
{
    RoaringBitmap result(roaring - rhs.roaring);
    return result;
}

void RoaringBitmap::invalidateBulk() noexcept
{
    bulk = roaring::BulkContext();
}

} // namespace SVF