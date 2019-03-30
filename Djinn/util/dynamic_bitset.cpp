#include "dynamic_bitset.h"

namespace djinn::util {
	DynamicBitset::DynamicBitset(size_t numBits, bool initialValue):
		m_Size(numBits)
	{
		m_Bits = std::make_unique<Piece[]>(numPieces(numBits));
	}

	bool DynamicBitset::test(int index) const noexcept {
	}

	void DynamicBitset::set(int index, bool value) noexcept {
	}

	void DynamicBitset::set(int from, int to, bool value) noexcept {
	}

	int DynamicBitset::findNext(int from, int to, bool value) noexcept {
	}

	constexpr DynamicBitset::Piece DynamicBitset::allBits(bool set) noexcept {
	}

	constexpr DynamicBitset::Piece DynamicBitset::bitMask(int select) noexcept {
	}

	size_t DynamicBitset::numPieces(size_t numBits) noexcept {
	}

	DynamicBitset::Piece& DynamicBitset::piece(int index) noexcept {
	}

	const DynamicBitset::Piece& DynamicBitset::piece(int index) const noexcept {
	}

	std::ostream& operator << (std::ostream& os, const DynamicBitset& db) {
	}
}