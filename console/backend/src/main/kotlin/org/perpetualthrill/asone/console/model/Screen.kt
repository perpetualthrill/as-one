package org.perpetualthrill.asone.console.model

import org.perpetualthrill.asone.console.model.ScreenConstants.BPM_AREA_WIDTH
import org.perpetualthrill.asone.console.model.ScreenConstants.CHAR_HEIGHT
import org.perpetualthrill.asone.console.model.ScreenConstants.CHAR_WIDTH
import org.perpetualthrill.asone.console.model.ScreenConstants.SCOREBOARD_ARRAY_SIZE
import org.perpetualthrill.asone.console.model.ScreenConstants.bignumTypeface
import org.perpetualthrill.asone.console.model.ScreenConstants.locationLookupTable

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
            for (i in 0 until locationLookupTable.size) {
                val x = locationLookupTable[i]["x"]
                val y = locationLookupTable[i]["y"]
                if (null != x && null != y) {
                    array[i * 3] = arrayXY[x][y].r
                    array[i * 3 + 1] = arrayXY[x][y].g
                    array[i * 3 + 2] = arrayXY[x][y].b
                }
            }
            return array
        }

    companion object {
        fun renderBPMCharacters(value: String): Array<UByteArray> {
            val charCount = value.length
            val spacerCount = if (charCount > 1) charCount - 1 else 0

            // First, build out full-size array
            val array = Array(CHAR_WIDTH * charCount + spacerCount, { UByteArray(CHAR_HEIGHT) })
            for (i in 0 until charCount) {
                val character = value[i]
                val glyph = bignumTypeface[character] ?: continue // null = leave blank
                val xOffset = i * CHAR_WIDTH + i // the +i is spacers
                for (x in 0 until CHAR_WIDTH) {
                    for (y in 0 until CHAR_HEIGHT) {
                        array[x + xOffset][y] = (255 * glyph[y][x]).toUByte()
                    }
                }
            }

            // Then, truncate or embiggen it as needed
            return when {
                array.size > BPM_AREA_WIDTH -> array.slice(IntRange(array.size - BPM_AREA_WIDTH, array.size - 1)).toTypedArray()
                array.size < BPM_AREA_WIDTH -> {
                    val leftpad = Array(BPM_AREA_WIDTH - array.size, { UByteArray(CHAR_HEIGHT) })
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
