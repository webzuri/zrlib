#include <assert.h>
#include <zrlib/base/ArrayOp.h>
#include <zrlib/base/Bits.h>

// ============================================================================

/**
 * Adapt the bits offset (and the position) if the position refer to a bit in another offset.
 */
#define ADJUST_POS(bits, pos) \
if (pos >= ZRBITS_NBOF) \
{ \
	bits += pos / ZRBITS_NBOF; \
	pos %= ZRBITS_NBOF; \
}

// ============================================================================
// GETMASK
// ============================================================================

ZRBits ZRBits_getLMask(size_t nbBits)
{
	assert(nbBits <= ZRBITS_NBOF);

	if (nbBits == 0)
		return (ZRBits)0;
	if (nbBits == ZRBITS_NBOF)
		return ZRBITS_MASK_FULL;

	ZRBits ret = ZRBITS_MASK_1L;

	while (--nbBits)
	{
		ret >>= 1;
		ret |= ZRBITS_MASK_1L;
	}
	return ret;
}

ZRBits ZRBits_getRMask(size_t nbBits)
{
	assert(nbBits <= ZRBITS_NBOF);

	if (nbBits == 0)
		return (ZRBits)0;
	if (nbBits == ZRBITS_NBOF)
		return ZRBITS_MASK_FULL;

	ZRBits ret = ZRBITS_MASK_1R;

	while (--nbBits)
	{
		ret <<= 1;
		ret |= ZRBITS_MASK_1R;
	}
	return ret;
}

ZRBits ZRBits_getMask(size_t nb1, bool toTheRight)
{
	if (toTheRight)
		return ZRBits_getRMask(nb1);

	return ZRBits_getLMask(nb1);
}

// ============================================================================

void ZRBits_cpack(ZRBits * bits, size_t nbBits, char * source, size_t sourceSize)
{
	size_t i = 0;

	while (sourceSize--)
	{
		ZRBits_setBitsFromTheRight(bits, i, nbBits, *source);
		source++;
		i += nbBits;
	}
}

/**
 * @param ZRBits *pack The destination of the packing
 * @param size_t nbBits Number of bit to copy for each ZRBits
 * @param unsigned int *source which contains the bits to get
 * @param size_t sourceSize The size of the source.
 */
void ZRBits_pack(ZRBits * restrict bits, size_t nbBits, ZRBits * restrict source, size_t sourceSize)
{
	size_t i = 0;

	while (sourceSize--)
	{
		ZRBits_setBitsFromTheRight(bits, i, nbBits, *source);
		source++;
		i += nbBits;
	}
}

// ============================================================================

void ZRBits_setBit(ZRBits *bits, size_t pos, bool bit)
{
	ADJUST_POS(bits, pos);
	const ZRBits mask = (ZRBITS_MASK_1L >> pos);

	if (bit)
		*bits |= mask;
	else
		*bits &= ~mask;
}

void ZRBits_setBitsFromTheRight(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source)
{
	ZRBits_setBits(bits, pos, nbBits, source, true);
}

void ZRBits_setBitsFromTheLeft(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source)
{
	ZRBits_setBits(bits, pos, nbBits, source, false);
}

void ZRBits_setBits(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source, bool fromTheRight)
{
	ADJUST_POS(bits, pos);
	size_t const nbAddPos = nbBits + pos;
	ZRBits const lmask = ZRBits_getLMask(nbBits);

	// We align to the left
	if (fromTheRight)
		source <<= ZRBITS_NBOF - nbBits;

	source &= ZRBits_getLMask(nbBits);
	*bits = (*bits & ~(lmask >> pos)) | source >> pos;

	if (nbAddPos > ZRBITS_NBOF)
	{
		size_t const finalBits = (pos + nbBits) % ZRBITS_NBOF;
		bits++;
		*bits = (*bits & (lmask << (ZRBITS_NBOF - finalBits))) | source << (nbBits - finalBits);
	}
}

bool ZRBits_getBit(ZRBits const *bits, size_t pos)
{
	ADJUST_POS(bits, pos);
	return (bool)(*bits & (ZRBITS_MASK_1L >> pos));
}

void ZRBits_getBits(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out)
{
	ZRBits_copy(bits, pos, nbBits, out, 0);
}

// ============================================================================
// COPY
// ============================================================================

static inline void ZRBits_copy_posEQOutPos(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	size_t const nbAddPos = nbBits + pos;
	size_t const nbAddOutPos = nbBits + outPos;

	if (pos == 0)
	{
		size_t const nbZRBits = nbBits / ZRBITS_NBOF;
		size_t const finalBits = nbBits % ZRBITS_NBOF;

		if (nbZRBits)
		{
			memcpy(out, bits, nbZRBits * sizeof(ZRBits));
			out += nbZRBits;
			bits += nbZRBits;
		}

		if (!finalBits)
			return;

		*out = *bits & ZRBits_getLMask(finalBits);
	}
	// The selection stay inside a ZRBits object
	else if (nbAddPos <= ZRBITS_NBOF)
	{
		ZRBits const mask = ZRBits_getLMask(nbBits) >> pos;
		*out = *bits & mask;
	}
	else
	{
		size_t const nbZRBits = (nbBits - (ZRBITS_NBOF - pos)) / ZRBITS_NBOF;
		size_t const finalBits = (nbBits + pos) % ZRBITS_NBOF;

		*out = *bits & ZRBits_getRMask(ZRBITS_NBOF - pos);
		out++;
		bits++;

		if (nbZRBits)
		{
			memcpy(out, bits, nbZRBits * sizeof(ZRBits));
			out += nbZRBits;
			bits += nbZRBits;
		}

		if (!finalBits)
			return;

		*out = *bits & ZRBits_getLMask(finalBits);
	}
}

static inline void ZRBits_copy_posGTOutPos(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	size_t const nbAddPos = nbBits + pos;
	size_t const nbAddOutPos = nbBits + outPos;
	size_t const posSubOutPos = pos - outPos;

	// The selection stay inside a ZRBits object
	if (nbAddPos <= ZRBITS_NBOF)
	{
		*out = (*bits & (ZRBits_getLMask(nbBits) >> pos)) << posSubOutPos;
	}
	// The result is store in only one ZRBits
	else if (nbAddOutPos <= ZRBITS_NBOF)
	{
		size_t const firstPart1Size = ZRBITS_NBOF - pos;
		size_t const firstPart2Size = nbBits - firstPart1Size;
		ZRBits const firstPart1Mask = ZRBits_getRMask(firstPart1Size);
		ZRBits const firstPart2Mask = ZRBits_getLMask(firstPart2Size);

		*out = ((*bits & firstPart1Mask) << posSubOutPos) | ((*(bits + 1) & firstPart2Mask) >> (firstPart1Size + outPos));
	}
	// The most general case
	else
	{
		size_t const firstPart1Size = ZRBITS_NBOF - pos;
		size_t const firstPart2Size = ZRBITS_NBOF - outPos - firstPart1Size;
		size_t const firstPartTotalSize = firstPart1Size + firstPart2Size;
		ZRBits const firstPart1Mask = ZRBits_getRMask(firstPart1Size);
		ZRBits const firstPart2Mask = ZRBits_getLMask(firstPart2Size);

		*out = ((*bits & firstPart1Mask) << posSubOutPos) | ((*(bits + 1) & firstPart2Mask) >> (firstPart1Size + outPos));
		bits++;
		out++;

		size_t nbZRBits = (nbBits - firstPartTotalSize) / ZRBITS_NBOF;
		size_t const finalBits = nbBits % ZRBITS_NBOF;

		size_t const intermPart1Size = ZRBITS_NBOF - firstPart2Size;
		size_t const intermPart2Size = firstPart2Size;
		ZRBits const intermPart1Mask = ~firstPart2Mask;
		ZRBits const intermPart2Mask = firstPart2Mask;

		if (nbZRBits)
		{
			while (nbZRBits--)
			{
				*out = (*bits & intermPart1Mask) << intermPart2Size | (*(bits + 1) & intermPart2Mask) >> intermPart1Size;
				bits++;
				out++;
			}
		}

		if (!finalBits)
			return;

		*out = (*bits & (ZRBits_getLMask(finalBits) >> intermPart2Size)) << intermPart2Size;
	}
}
static inline void ZRBits_copy_posLTOutPos(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	size_t const nbAddOutPos = nbBits + outPos;
	size_t outPosSubPos = outPos - pos;

	// The result is stored in only one ZRBits
	if (nbAddOutPos <= ZRBITS_NBOF)
		*out = (*bits & (ZRBits_getLMask(nbBits) >> pos)) >> outPosSubPos;
	else
	{
		size_t const firstPart1Size = ZRBITS_NBOF - outPos;
		ZRBits const firstPart1Mask = ZRBits_getLMask(firstPart1Size);

		*out = (*bits & (firstPart1Mask >> pos)) >> outPosSubPos;
		ZRBits_copy_posGTOutPos(bits, pos + firstPart1Size, nbBits - firstPart1Size, out + 1, 0);
	}
}

void ZRBits_copy(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	ADJUST_POS(bits, pos);
	ADJUST_POS(out, outPos);

	if (pos == outPos)
		ZRBits_copy_posEQOutPos(bits, pos, nbBits, out, outPos);
	else if (pos > outPos)
		ZRBits_copy_posGTOutPos(bits, pos, nbBits, out, outPos);
	else
		ZRBits_copy_posLTOutPos(bits, pos, nbBits, out, outPos);
}

// ============================================================================
// SHIFT
// ============================================================================

static inline void ZRBits_inArrayLShift_(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	size_t const leftPartBits = ZRBITS_NBOF - shift;
	ZRBits const leftPartMask = ZRBits_getRMask(leftPartBits);
	ZRBits const leftPartComplementMask = ~leftPartMask;

	if (nbZRBits > 1)
	{
		while (--nbZRBits)
		{
			*bits = ((*bits & leftPartMask) << shift) | ((*(bits + 1) & leftPartComplementMask) >> leftPartBits);
			bits++;
		}
	}
	*bits <<= shift;
}

static inline void ZRBits_inArrayRShift_(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	size_t const rightPartBits = ZRBITS_NBOF - shift;
	ZRBits const rightPartMask = ZRBits_getRMask(rightPartBits);
	ZRBits const rightPartComplementMask = ~rightPartMask;

	if (nbZRBits > 1)
	{
		bits += nbZRBits - 1;

		while (--nbZRBits)
		{
			*bits = ((*bits & rightPartMask) >> shift) | ((*(bits - 1) & rightPartComplementMask) << rightPartBits);
			bits--;
		}
	}
	*bits >>= shift;
}

void ZRBits_inArrayRShift(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	ZRBits_inArrayRShift_(bits, nbZRBits, shift);
}

void ZRBits_inArrayLShift(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	ZRBits_inArrayLShift_(bits, nbZRBits, shift);
}

void ZRBits_inArrayShift(ZRBits *bits, size_t nbZRBits, size_t shift, size_t toTheRight)
{
	assert(nbZRBits > 0);

	if (shift % CHAR_BIT == 0)
	{
		shift /= CHAR_BIT;
		char *bits0fill;

		if (toTheRight)
			bits0fill = (char*)bits;
		else
			bits0fill = (char*)bits + nbZRBits * (sizeof(*bits) / sizeof(char)) - shift;

		ZRBits zeros = ZRBITS_0;
		ZRARRAYOP_SHIFT(bits, sizeof(char), nbZRBits * sizeof(*bits) / sizeof(char), shift, toTheRight);
		ZRARRAYOP_FILL(bits0fill, sizeof(char), shift, &zeros);
		return;
	}

	if (toTheRight)
		ZRBits_inArrayRShift_(bits, nbZRBits, shift);
	else
		ZRBits_inArrayLShift_(bits, nbZRBits, shift);
}
