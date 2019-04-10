#pragma once

#include <memory>
#include <iosfwd>

namespace djinn::util {
    // similar to std::bitset, except this is dynamically sized
    // also somewhat similar to std::vector<bool>, with less foot guns
    // could be optimized further

	class DynamicBitset {
	public:
		DynamicBitset(size_t numBits, bool initialValue = false);

		bool   test(size_t index)       const noexcept;
        size_t count(bool value = true) const noexcept;
        size_t size()                   const noexcept;
		
		void set(size_t index,           bool value = true) noexcept;
		void set(size_t from, size_t to, bool value       ) noexcept; // [from, to)

		size_t findNext(size_t from, size_t to, bool value) noexcept; // returns (to) if the value was not found

		friend std::ostream& operator<<(std::ostream&, const DynamicBitset&);

	private:
		using Piece = uint32_t;
		static constexpr int bitsPerPiece = static_cast<int>(sizeof(Piece) * CHAR_BIT);

		static constexpr Piece allBits(bool set) noexcept;
		static constexpr Piece bitMask(size_t select) noexcept;
		
		static size_t numPieces(size_t numBits) noexcept;

		      Piece& piece(size_t index)       noexcept;
		const Piece& piece(size_t index) const noexcept;

		std::unique_ptr<Piece[]> m_Bits;
		size_t                   m_Size;
	};
}
