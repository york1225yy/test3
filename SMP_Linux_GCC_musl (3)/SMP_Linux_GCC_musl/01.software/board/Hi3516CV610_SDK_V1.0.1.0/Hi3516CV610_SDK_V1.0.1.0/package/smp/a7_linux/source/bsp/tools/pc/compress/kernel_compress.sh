#!/bin/sh

########  header formet ###############
# [ compress size ] [   orig size   ] [  magic string ]
# [     4Bytes    ] [     4Bytes    ] [     8Bytes    ]
# [  header2.bin  ] [  header1.bin  ] [  header3.bin  ]

InputImage=$1

#
# 4Byte padding for uImage,if use hardware gzip + uImage nocopy
#
if [ $# == 2 ]; then
	if [ $2 == "-p" ]; then
		echo -n "FFFF" | xxd -p | xxd -r -ps > pad.bin
		cat pad.bin ${InputImage} > ${InputImage}.tmp
		rm ${InputImage}
		rm pad.bin
		mv ${InputImage}.tmp ${InputImage}
	fi
fi

if [ $InputImage ]; then
	size=$(wc -c $InputImage | awk '{print $1}')
	size=$(echo $size | xargs printf "%08x")
	size=$(echo ${size:6:2}${size:4:2}${size:2:2}${size:0:2})
	echo $size | xxd -r -ps > header1.bin

	./gzip ${InputImage}

	size=$(wc -c ${InputImage}.gz | awk '{print $1}')
	size=$(echo $size | xargs printf "%08x")
	size=$(echo ${size:6:2}${size:4:2}${size:2:2}${size:0:2})
	echo $size | xxd -r -ps > header2.bin

	echo -n "gziphead" | xxd -p | xxd -r -ps > header3.bin

	mv ${InputImage}.gz ${InputImage}.tmp.gz

	cat header2.bin header1.bin header3.bin ${InputImage}.tmp.gz > ${InputImage}.gz

	rm header1.bin header2.bin header3.bin ${InputImage}.tmp.gz
else
	echo "Usage: ./gzip_compress.sh <imageName> [OPTION]"
	echo "valid OPTION:"
	echo "    -p: padding 4 byte data for bin"
	echo "Example: ./gzip_compress.sh uImage"
	echo "Example: ./gzip_compress.sh uImage -p"
fi
