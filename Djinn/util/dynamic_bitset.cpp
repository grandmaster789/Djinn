#include "dynamic_bitset.h"
#include <algorithm>
#include <cassert>
#include <ostream>

namespace djinn::util {
	DynamicBitset::DynamicBitset(size_t numBits, bool initialValue): m_Size(numBits) {
		m_Bits = std::make_unique<Piece[]>(numPieces(numBits));

		set(0, numBits - 1, initialValue);
	}

	bool DynamicBitset::test(size_t index) const noexcept {
		return (piece(index) & bitMask(index)) != Piece(0);
	}

	size_t DynamicBitset::count(bool value) const noexcept {
		size_t result = 0;

		for (size_t i = 0; i < m_Size; ++i)
			if (test(i) == value) ++result;

		return result;
	}

	size_t DynamicBitset::size() const noexcept {
		return m_Size;
	}

	void DynamicBitset::set(size_t index, bool value) noexcept {
		if (value)
			piece(index) |= bitMask(index);
		else
			piece(index) &= ~bitMask(index);
	}

	void DynamicBitset::set(size_t from, size_t to, bool value) noexcept {
		assert(from <= to);  // make sure the arguments are in low-to-high order

		if (from == to) return;

		const size_t fromPiece = from / bitsPerPiece;
		const size_t fromBit   = from % bitsPerPiece;
		const size_t toPiece   = to / bitsPerPiece;
		const size_t toBit     = to % bitsPerPiece;

		// whoo pointer arithmetic
		Piece* raw = m_Bits.get() + fromPiece;

		// if we start somewhere in the middle of the piece, perform masking on that piece
		if (fromBit != 0) {
			Piece mask = (Piece(1) << fromBit) - 1;

			if (fromPiece == toPiece) mask |= ~((Piece(1) << toBit) - 1);

			if (value)
				*raw |= ~mask;
			else
				*raw &= mask;

			if (fromPiece == toPiece) return;

			++raw;
		}

		// apply bulk operations on entire pieces
		raw = std::fill_n(raw, m_Bits.get() + toPiece - raw, allBits(value));

		// if we end somewhere in the middle of a piece, do masking again
		if (toBit != 0) {
			Piece mask = (Piece(1) << toBit) - 1;

			if (value)
				*raw |= mask;
			else
				*raw &= ~mask;
		}
	}

	size_t DynamicBitset::findNext(size_t from, size_t to, bool value) noexcept {
		assert(from <= to);  // make sure the arguments are in low-to-high order

		if (from == to) return to;  // not found

		const size_t fromPiece = from / bitsPerPiece;
		const size_t fromBit   = from % bitsPerPiece;
		const size_t toPiece   = to / bitsPerPiece;
		const size_t toBit     = to % bitsPerPiece;

		Piece* raw = m_Bits.get() + fromPiece;

		// if we start somewhere in the middle of the piece, perform masking on that piece
		if (fromBit != 0) {
			Piece mask = (Piece(1) << fromBit) - 1;

			if (fromPiece == toPiece) mask |= ~((Piece(1) << toBit) - 1);

			mask = ~mask;

			if ((value && (*raw & mask) != Piece(0)) ||  // test for true
			    (!value && (~*raw & mask) != Piece(0))   // test for false
			) {
				// found a match in the first piece, scan individual bits
				while (test(from) != value) ++from;

				return from;
			}

			if (fromPiece == toPiece) return to;  // not found

			++raw;
		}

		// apply bulk operations on entire pieces
		raw = std::find_if(
		    raw, m_Bits.get() + toPiece, [=](Piece p) { return (p != allBits(value)); });

		// if we found a match with the bulk operation, scan the piece
		if (raw != m_Bits.get() + toPiece) {
			from = (raw - m_Bits.get()) * bitsPerPiece;  // convert raw pointer to index

			while (test(from) != value) ++from;

			return from;
		}

		// if we end somewhere in the middle of a piece, do masking again
		if (toBit != 0) {
			Piece mask = (Piece(1) << toBit) - 1;

			if ((value && (*raw & mask) != Piece(0)) || (!value && (~*raw & mask) != Piece(0))) {
				from = (raw - m_Bits.get()) * bitsPerPiece;

				while (test(from) != value) ++from;

				return from;
			}
		}

		return to;
	}

	constexpr DynamicBitset::Piece DynamicBitset::allBits(bool set) noexcept {
		if (set)
			return ~Piece(0);
		else
			return Piece(0);
	}

	constexpr DynamicBitset::Piece DynamicBitset::bitMask(size_t select) noexcept {
		return Piece(1) << (select % bitsPerPiece);
	}

	size_t DynamicBitset::numPieces(size_t numBits) noexcept {
		return (numBits + bitsPerPiece - 1) / bitsPerPiece;
	}

	DynamicBitset::Piece& DynamicBitset::piece(size_t index) noexcept {
		return m_Bits[index / bitsPerPiece];
	}

	const DynamicBitset::Piece& DynamicBitset::piece(size_t index) const noexcept {
		return m_Bits[index / bitsPerPiece];
	}

	std::ostream& operator<<(std::ostream& os, const DynamicBitset& db) {
		for (size_t i = 0; i < db.m_Size; ++i) {
			os << (db.test(i) ? "1" : "0");

			if (i % 8 == 7) os << ' ';

			if (i % 64 == 63) os << '\n';
		}

		return os;
	}
}  // namespace djinn::util
