#pragma once

#include <memory>
#include <iosfwd>

namespace djinn::util {
	class DynamicBitset {
	public:
		DynamicBitset(size_t numBits, bool initialValue = false);

		bool test(int index) const noexcept;
		
		void set(int index, bool value) noexcept;
		void set(int from, int to, bool value) noexcept; // [from, to)

		int findNext(int from, int to, bool value) noexcept;

		friend std::ostream& operator<<(std::ostream&, const DynamicBitset&);

	private:
		using Piece = uint32_t;
		static constexpr int bitsPerPiece = static_cast<int>(sizeof(Piece) * CHAR_BIT);

		static constexpr Piece allBits(bool set) noexcept;
		static constexpr Piece bitMask(int select) noexcept;
		
		static size_t numPieces(size_t numBits) noexcept;

		      Piece& piece(int index)       noexcept;
		const Piece& piece(int index) const noexcept;

		std::unique_ptr<Piece[]> m_Bits;
		size_t                   m_Size;
	};
}
