DOXYFILE = "mcss-Doxyfile"
THEME_COLOR = "#cb4b16"
FAVICON = "enviroDIY_Favicon.png"
LINKS_NAVBAR1 = [
    (
        "About",
        "index",
        [
            ('<a href="page_calibration.html">Calibration</a>',),
            ('<a href="change_log.html">ChangeLog</a>',),
        ],
    ),
    ('<a href="classyosemitech.html">The yosemitech Class</a>',[]),
    (
        "Examples",
        "page_the_examples",
        [
            ('<a href="example_get_values.html">Reading Sensor Values</a>',),
            (
                '<a href="example_display_values.html">Displaying Values to a Screen</a>',
            ),
        ],
    ),
    (
        "Source Files",
        "files",
        [],
    ),
]
LINKS_NAVBAR2 = []
VERSION_LABELS = True
CLASS_INDEX_EXPAND_LEVELS = 2

STYLESHEETS = [
    "css/m-EnviroDIY+documentation.compiled.css",
]
EXTRA_FILES = ["gp-desktop-logo.png", "gp-mobile-logo.png", "gp-scrolling-logo.png", "clipboard.js"]
DESKTOP_LOGO = "gp-desktop-logo.png"
MOBILE_LOGO = "gp-mobile-logo.png"
SCROLLING_LOGO = "gp-scrolling-logo.png"
