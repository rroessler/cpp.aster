# - PRELUDE INCLUDES - #

include_guard()

# - PRELUDE SETUP - #

# prepare the baseline variables
set(ASTER_TARGET_NAME "aster")
set(ASTER_TARGET_TITLE "Aster")

# declare some additional CXX details
set(ASTER_CXX_STANDARD 23)

# prepare the baseline versioning to be used
set(ASTER_VERSION_MAJOR 1)
set(ASTER_VERSION_MINOR 1)
set(ASTER_VERSION_PATCH 3)

# combine all the versioning details together now
set(ASTER_VERSION_SHORT "${ASTER_VERSION_MAJOR}.${ASTER_VERSION_MINOR}.${ASTER_VERSION_PATCH}")
