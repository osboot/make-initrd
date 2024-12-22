#!/bin/bash -efu
# SPDX-License-Identifier: GPL-3.0-or-later

[ -d .wiki ] ||
	git clone https://github.com/osboot/make-initrd.wiki.git .wiki

copy_readme()
{
	local subname prefix wikiprefix

	subname="$1"; shift
	prefix="$1"; shift
	wikiprefix="$1"; shift

	find "$subname" -mindepth 2 -name README.md -printf '%p\n' |
		sort |
	while read -r file; do
		title=$(grep -m1 -e "^# $prefix " "$file") ||
			continue

		name="${file#$subname}"
		name="${name%/README.md}"

		cp -f --  "$file" ".wiki/${wikiprefix}${name}.md"

		printf '* [%s](%s)\n' "${title#* $prefix }" "${wikiprefix}${name}"
	done
}

find Documentation/ \
	\( \
		   -name '*.md'             -a \
		\! -name 'README.md'        -a \
		\! -path '*/manpages/*'        \
		\! -path '*/presentation/*'   \
	\) \
	-exec cp -f -t .wiki -- '{}' '+'

cp -f -- Documentation/README.md .wiki/Home.md
cp -f -- features/README.md .wiki/Features.md

echo >> .wiki/Features.md
echo >> .wiki/GuessConfiguration.md

copy_readme features/ "Feature:"      "feature-" >> .wiki/Features.md
copy_readme guess/    "Guess module:" "guess-"   >> .wiki/GuessConfiguration.md

find .wiki \
	-name '*.md' \
	-exec sed -r -i \
			-e 's,\((\.\./)*Documentation/,(,g' \
			-e 's,\(guess/,(guess-,g' \
			-e 's,\(features/,(feature-,g' \
			-e 's,/README\.md\),),g' \
			-e 's,\.md\),),g' \
			-e 's,\.md#,#,g' \
		'{}' '+'
