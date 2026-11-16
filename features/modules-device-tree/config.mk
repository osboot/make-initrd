# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,add-modules depmod-image)

# These filters are complementary to each other.
GUESS_DT_NAMES ?=
GUESS_DT_COMPATIBLES ?=
