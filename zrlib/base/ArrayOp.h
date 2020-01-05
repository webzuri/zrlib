/**
 * @author zuri
 * @date samedi 28 juin 2014, 13:27:45 (UTC+0200)
 */

#ifndef ZRMEMORY_ARRAY_OP_H
#define ZRMEMORY_ARRAY_OP_H

#include <zrlib/config.h>
#include <zrlib/base/MemoryOp.h>

#include <zrlib/syntax_pad.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// ============================================================================

#define ZRARRAYOP_GET(offset, objSize, pos) \
	((char*)(offset) + (pos) * (objSize))

#define ZRARRAYOP_SET(offset, objSize, pos, source) \
	memcpy(ZRARRAYOP_GET(offset, objSize, pos), (source), (objSize))

#define ZRARRAYOP_SWAP(offset, objSize, posa, posb) \
	ZRMemoryOp_swap(ZRARRAYOP_GET(offset, objSize, posa), ZRARRAYOP_GET(offset, objSize, posb), (objSize))

#define ZRARRAYOP_SHIFT(offset, objSize, nbObj, shift, toTheRight) \
	ZRMemoryOp_shift((offset), ZRARRAYOP_GET(offset, objSize, nbObj), (shift)  * (objSize), (toTheRight))

#define ZRARRAYOP_ROTATE(offset, objSize, nbObj, rotate, toTheRight) \
	ZRMemoryOp_rotate((offset), ZRARRAYOP_GET(offset, objSize, nbObj), (rotate) * (objSize), (toTheRight))

#define ZRARRAYOP_FILL(offset, objSize, nbObj, object) \
	ZRMemoryOp_fill(offset, object, objSize, nbObj);

#define ZRARRAYOP_CPY(offset, objSize, nbObj, source) \
	memcpy((offset), (source), (objSize) * (nbObj))

#define ZRARRAYOP_MOVE(offset, objSize, nbObj, source) \
	memmove((offset), (source), (objSize) * (nbObj))

#define ZRARRAYOP_DEPLACE(offset, objSize, nbObj, source) \
	ZRMemoryOp_deplace((offset), (source), (objSize) * (nbObj))

static inline void ZRARRAYOP_REVERSE(void *offset, size_t objSize, size_t nbObj)
{
	char *a = offset;
	char *b = (char*)offset + (objSize * (nbObj - 1));
	char buffer[objSize];

	nbObj >>= 1;

	while (nbObj--)
	{
		ZRMemoryOp_swapB(a, b, objSize, buffer);
		a += objSize;
		b -= objSize;
	}
}

// ============================================================================

void*_ ZRArrayOp_get(__ void _________*offset, size_t objSize, size_t pos);
void _ ZRArrayOp_set(__ void *restrict offset, size_t objSize, size_t pos, void *restrict source);
void _ ZRArrayOp_swap(_ void _________*offset, size_t objSize, size_t posa, size_t posb);

void ZRArrayOp_fill(____ void *restrict offset, size_t objSize, size_t nbObj, void *restrict object);
void ZRArrayOp_cpy(_____ void *restrict offset, size_t objSize, size_t nbObj, void *restrict source);
void ZRArrayOp_move(____ void _________*offset, size_t objSize, size_t nbObj, void _________*source);
void ZRArrayOp_deplace(_ void _________*offset, size_t objSize, size_t nbObj, void _________*source);

void ZRArrayOp_shift(___ void *offset, size_t objSize, size_t nbObj, size_t shift,_ bool toTheRight);
void ZRArrayOp_rotate(__ void *offset, size_t objSize, size_t nbObj, size_t rotate, bool toTheRight);
void ZRArrayOp_reverse(_ void *offset, size_t objSize, size_t nbObj);

#endif