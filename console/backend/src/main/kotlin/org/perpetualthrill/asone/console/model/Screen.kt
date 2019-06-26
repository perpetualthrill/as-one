package org.perpetualthrill.asone.console.model

// Lookup table. Each entry in this array corresponds to a location on the
// screen. The scoreboard gets filled by sampling the screen, which can be
// done by walking this table
private val SCOREBOARD_LOCATIONS = arrayOf(
    mapOf("x" to 30, "y" to 5),
    mapOf("x" to 29, "y" to 5),
    mapOf("x" to 28, "y" to 5),
    mapOf("x" to 27, "y" to 5),
    mapOf("x" to 27, "y" to 6),
    mapOf("x" to 27, "y" to 7),
    mapOf("x" to 27, "y" to 8),
    mapOf("x" to 28, "y" to 8),
    mapOf("x" to 29, "y" to 8),
    mapOf("x" to 30, "y" to 8),
    mapOf("x" to 30, "y" to 7),
    mapOf("x" to 30, "y" to 6),
    mapOf("x" to 30, "y" to 4),
    mapOf("x" to 30, "y" to 3),
    mapOf("x" to 30, "y" to 2),
    mapOf("x" to 29, "y" to 2),
    mapOf("x" to 28, "y" to 2),
    mapOf("x" to 27, "y" to 2),
    mapOf("x" to 27, "y" to 3),
    mapOf("x" to 27, "y" to 4),
    mapOf("x" to 25, "y" to 5),
    mapOf("x" to 24, "y" to 5),
    mapOf("x" to 23, "y" to 5),
    mapOf("x" to 22, "y" to 5),
    mapOf("x" to 22, "y" to 6),
    mapOf("x" to 22, "y" to 7),
    mapOf("x" to 22, "y" to 8),
    mapOf("x" to 23, "y" to 8),
    mapOf("x" to 24, "y" to 8),
    mapOf("x" to 25, "y" to 8),
    mapOf("x" to 25, "y" to 7),
    mapOf("x" to 25, "y" to 6),
    mapOf("x" to 25, "y" to 4),
    mapOf("x" to 25, "y" to 3),
    mapOf("x" to 25, "y" to 2),
    mapOf("x" to 24, "y" to 2),
    mapOf("x" to 23, "y" to 2),
    mapOf("x" to 22, "y" to 2),
    mapOf("x" to 22, "y" to 3),
    mapOf("x" to 22, "y" to 4),
    mapOf("x" to 20, "y" to 2),
    mapOf("x" to 20, "y" to 3),
    mapOf("x" to 20, "y" to 4),
    mapOf("x" to 20, "y" to 5),
    mapOf("x" to 20, "y" to 6),
    mapOf("x" to 20, "y" to 7),
    mapOf("x" to 20, "y" to 8),
    mapOf("x" to 17, "y" to 9),
    mapOf("x" to 16, "y" to 9),
    mapOf("x" to 15, "y" to 9),
    mapOf("x" to 14, "y" to 9),
    mapOf("x" to 13, "y" to 9),
    mapOf("x" to 13, "y" to 8),
    mapOf("x" to 14, "y" to 8),
    mapOf("x" to 15, "y" to 8),
    mapOf("x" to 16, "y" to 8),
    mapOf("x" to 17, "y" to 8),
    mapOf("x" to 17, "y" to 7),
    mapOf("x" to 16, "y" to 7),
    mapOf("x" to 15, "y" to 7),
    mapOf("x" to 14, "y" to 7),
    mapOf("x" to 13, "y" to 7),
    mapOf("x" to 18, "y" to 2),
    mapOf("x" to 17, "y" to 2),
    mapOf("x" to 16, "y" to 2),
    mapOf("x" to 16, "y" to 3),
    mapOf("x" to 16, "y" to 4),
    mapOf("x" to 17, "y" to 4),
    mapOf("x" to 18, "y" to 4),
    mapOf("x" to 18, "y" to 3),
    mapOf("x" to 18, "y" to 1),
    mapOf("x" to 18, "y" to 0),
    mapOf("x" to 17, "y" to 0),
    mapOf("x" to 16, "y" to 0),
    mapOf("x" to 16, "y" to 1),
    mapOf("x" to 14, "y" to 2),
    mapOf("x" to 13, "y" to 2),
    mapOf("x" to 12, "y" to 2),
    mapOf("x" to 12, "y" to 3),
    mapOf("x" to 12, "y" to 4),
    mapOf("x" to 13, "y" to 4),
    mapOf("x" to 14, "y" to 4),
    mapOf("x" to 14, "y" to 3),
    mapOf("x" to 14, "y" to 1),
    mapOf("x" to 14, "y" to 0),
    mapOf("x" to 13, "y" to 0),
    mapOf("x" to 12, "y" to 0),
    mapOf("x" to 12, "y" to 1),
    mapOf("x" to 10, "y" to 5),
    mapOf("x" to 9, "y" to 5),
    mapOf("x" to 8, "y" to 5),
    mapOf("x" to 7, "y" to 5),
    mapOf("x" to 7, "y" to 6),
    mapOf("x" to 7, "y" to 7),
    mapOf("x" to 7, "y" to 8),
    mapOf("x" to 8, "y" to 8),
    mapOf("x" to 9, "y" to 8),
    mapOf("x" to 10, "y" to 8),
    mapOf("x" to 10, "y" to 7),
    mapOf("x" to 10, "y" to 6),
    mapOf("x" to 10, "y" to 4),
    mapOf("x" to 10, "y" to 3),
    mapOf("x" to 10, "y" to 2),
    mapOf("x" to 9, "y" to 2),
    mapOf("x" to 8, "y" to 2),
    mapOf("x" to 7, "y" to 2),
    mapOf("x" to 7, "y" to 3),
    mapOf("x" to 7, "y" to 4),
    mapOf("x" to 5, "y" to 5),
    mapOf("x" to 4, "y" to 5),
    mapOf("x" to 3, "y" to 5),
    mapOf("x" to 2, "y" to 5),
    mapOf("x" to 2, "y" to 6),
    mapOf("x" to 2, "y" to 7),
    mapOf("x" to 2, "y" to 8),
    mapOf("x" to 3, "y" to 8),
    mapOf("x" to 4, "y" to 8),
    mapOf("x" to 5, "y" to 8),
    mapOf("x" to 5, "y" to 7),
    mapOf("x" to 5, "y" to 6),
    mapOf("x" to 5, "y" to 4),
    mapOf("x" to 5, "y" to 3),
    mapOf("x" to 5, "y" to 2),
    mapOf("x" to 4, "y" to 2),
    mapOf("x" to 3, "y" to 2),
    mapOf("x" to 2, "y" to 2),
    mapOf("x" to 2, "y" to 3),
    mapOf("x" to 2, "y" to 4),
    mapOf("x" to 0, "y" to 2),
    mapOf("x" to 0, "y" to 3),
    mapOf("x" to 0, "y" to 4),
    mapOf("x" to 0, "y" to 5),
    mapOf("x" to 0, "y" to 6),
    mapOf("x" to 0, "y" to 7),
    mapOf("x" to 0, "y" to 8)
)

private val SCOREBOARD_ARRAY_SIZE = SCOREBOARD_LOCATIONS.size * 3 // i.e. rgb triples

const val SCREEN_WIDTH = 31
const val SCREEN_HEIGHT = 10

const val CHAR_WIDTH = 4
const val CHAR_HEIGHT = 7

const val BIGNUM_WIDTH = 11

data class Color(
    val r: UByte,
    val g: UByte,
    val b: UByte,
    val a: UByte = 255u
)

data class Screen(
    val arrayXY: Array<Array<Color>> = arrayOf()
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as Screen

        if (!arrayXY.contentDeepEquals(other.arrayXY)) return false

        return true
    }

    override fun hashCode(): Int {
        return arrayXY.contentDeepHashCode()
    }

    val toAsOneScoreboard: UByteArray
        get() {
            val array = UByteArray(SCOREBOARD_ARRAY_SIZE)
            for (i in 0 until SCOREBOARD_LOCATIONS.size) {
                val x = SCOREBOARD_LOCATIONS[i]["x"]
                val y = SCOREBOARD_LOCATIONS[i]["y"]
                if (null != x && null != y) {
                    array[i * 3] = arrayXY[x][y].r
                    array[i * 3 + 1] = arrayXY[x][y].g
                    array[i * 3 + 2] = arrayXY[x][y].b
                }
            }
            return array
        }

    companion object {
        fun renderBigNums(value: String): Array<UByteArray> {
            val charCount = value.length
            val spacerCount = if (charCount > 1) charCount - 1 else 0

            // First, build out full-size array
            val array = Array(CHAR_WIDTH * charCount + spacerCount, { UByteArray(CHAR_HEIGHT) })
            for (i in 0 until charCount) {
                val character = value[i]
                val glyph = Typeface.map[character] ?: continue // null = leave blank
                val xOffset = i * CHAR_WIDTH + i // the +i is spacers
                for (x in 0 until CHAR_WIDTH) {
                    for (y in 0 until CHAR_HEIGHT) {
                        array[x + xOffset][y] = (255 * glyph[y][x]).toUByte()
                    }
                }
            }

            // Then, truncate or embiggen it as needed
            return when {
                array.size > BIGNUM_WIDTH -> array.slice(IntRange(array.size - BIGNUM_WIDTH, array.size - 1)).toTypedArray()
                array.size < BIGNUM_WIDTH -> {
                    val leftpad = Array(BIGNUM_WIDTH - array.size, { UByteArray(CHAR_HEIGHT) })
                    leftpad + array
                }
                else -> array
            }
        }

        fun toTestString(array: Array<UByteArray>): String {
            val builder = StringBuilder()
            for (y in 0 until array[0].size) {
                for (x in 0 until array.size) {
                    val char = if (array[x][y] > 0.toUByte()) '#' else ' '
                    builder.append(char)
                }
                builder.append('\n')
            }
            return builder.toString()
        }
    }
}
