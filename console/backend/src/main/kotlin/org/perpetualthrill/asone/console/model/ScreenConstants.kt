package org.perpetualthrill.asone.console.model

object ScreenConstants {

    val screenRect = Rect(
        Location(0,0),
        31,
        10
    )

    const val CHAR_WIDTH = 4
    const val CHAR_HEIGHT = 7

    const val BPM_AREA_WIDTH = 11

    // Start locations of bpm areas of screen
    val leftBPMRect = Rect(
        Location(0, 2),
        BPM_AREA_WIDTH,
        CHAR_HEIGHT
    )
    val rightBPMRect = leftBPMRect.copy(location = Location(20, 2))

    // Centered timer area
    val timerStart = Location(12, 0)
    val timerRect = Rect(timerStart, 7, 5)

    // Lookup table. Each entry in this array corresponds to a location on the
    // screen. The scoreboard gets filled by sampling the screen, which can be
    // done by walking this table
    val locationLookupTable = arrayOf(
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

    val SCOREBOARD_ARRAY_SIZE = locationLookupTable.size * 3 // i.e. rgb triples

    // Character to display layout map
    val bpmTypeface = mapOf(
        '0' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 1)
        ),
        '1' to arrayOf(
            arrayOf(0, 0, 1, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1)
        ),
        '2' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 1, 1, 1)
        ),
        '3' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 1, 1, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(1, 1, 1, 1)
        ),
        '4' to arrayOf(
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1)
        ),
        '5' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 1, 1, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(1, 1, 1, 1)
        ),
        '6' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 1)
        ),
        '7' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1)
        ),
        '8' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 1)
        ),
        '9' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 1, 1, 1)
        ),
        'A' to arrayOf(
            arrayOf(0, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1)
        ),
        'B' to arrayOf(
            arrayOf(1, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 0)
        ),
        'C' to arrayOf(
            arrayOf(0, 1, 1, 1),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(0, 1, 1, 1)
        ),
        'D' to arrayOf(
            arrayOf(1, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 0)
        ),
        'E' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 1, 1, 1)
        ),
        'F' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 1, 1, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0)
        ),
        'G' to arrayOf(
            arrayOf(0, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 1, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 1, 1, 0)
        ),
        'H' to arrayOf(
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1)
        ),
        'I' to arrayOf(
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1)
        ),
        'J' to arrayOf(
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 1, 1, 0)
        ),
        'K' to arrayOf(
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1)
        ),
        'L' to arrayOf(
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 1, 1, 1)
        ),
        'M' to arrayOf( // this sucks
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 1, 1, 0),
            arrayOf(0, 0, 0, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1)
        ),
        'N' to arrayOf(
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1)
        ),
        'O' to arrayOf(
            arrayOf(0, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 1, 1, 0)
        ),
        'P' to arrayOf(
            arrayOf(0, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0)
        ),
        'Q' to arrayOf(
            arrayOf(0, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 0),
            arrayOf(0, 1, 0, 1)
        ),
        'R' to arrayOf(
            arrayOf(1, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1)
        ),
        'S' to arrayOf(
            arrayOf(0, 1, 1, 1),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(0, 1, 1, 0),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(1, 1, 1, 0)
        ),
        'T' to arrayOf(
            arrayOf(0, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 1, 1, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(0, 1, 1, 0)
        ),
        'U' to arrayOf(
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 1, 1, 1)
        ),
        'V' to arrayOf(
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 1, 1, 0)
        ),
        'W' to arrayOf(
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 0)
        ),
        'X' to arrayOf(
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1)
        ),
        'Y' to arrayOf(
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 1, 1, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 1, 1, 0)
        ),
        'Z' to arrayOf(
            arrayOf(1, 1, 1, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 1, 1, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 0, 0, 0),
            arrayOf(1, 1, 1, 1)
        ),
        ' ' to arrayOf(
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 0)
        ),
        '?' to arrayOf(
            arrayOf(0, 1, 1, 0),
            arrayOf(1, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 1, 0),
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 1, 0)
        ),
        '!' to arrayOf(
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 1),
            arrayOf(0, 0, 0, 0),
            arrayOf(0, 0, 0, 1)
        )
    )

}
