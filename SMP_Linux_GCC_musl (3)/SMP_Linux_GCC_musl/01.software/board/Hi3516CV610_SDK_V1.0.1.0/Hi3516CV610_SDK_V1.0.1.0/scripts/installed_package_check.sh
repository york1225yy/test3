#!/bin/bash

apt_get_install_package="make libc6-i386 lib32z1 lib32stdc++6 zlib1g-dev libncurses5-dev \
	ncurses-term libncursesw5-dev g++ u-boot-tools texinfo gawk libssl-dev openssl bc p7zip-full gperf \
	bison flex diffutils git unzip libffi-dev libtool libfreetype6 fakeroot autopoint po4a zlib1g-dev \
	liblzo2-dev uuid-dev pkg-config automake texlive python3"

pip3_installed_package="wheel pycryptodome pyelftools cryptography"

installed_apt_package_check()
{
	ITEMS=$(echo $1)
	for ITEM in ${ITEMS[@]}; do
		if [ $ITEM == "python3" ];then
                        python3 --version|grep -wo "Python 3" > /dev/null
                else
                        echo $2|grep -o "ii\s\+$ITEM"  > /dev/null
                fi

		if [ $? -ne 0 ];then
			echo -e "\033[31m environment check $ITEM fail,please install it! \033[0m"
		else
			echo -e "\033[32m environment check $ITEM success \033[0m"
		fi
	done
}

installed_pip_package_check()
{
	ITEMS=$(echo $1)
	for ITEM in ${ITEMS[@]}; do
		echo $2|grep -wo $ITEM  > /dev/null

		if [ $? -ne 0 ];then
			echo -e "\033[31m environment check $ITEM fail,please install it! \033[0m"
		else
			echo -e "\033[32m environment check $ITEM success \033[0m"
		fi
	done
}

echo -e "\033[33m \nstart check apt_get installed package: \033[0m"
installed=$(echo `dpkg -l | grep '^ii'`)
installed_apt_package_check "$apt_get_install_package" "$installed"

echo -e "\033[33m \nstart check pip3 installed package: \033[0m"
installed=$(echo `pip3 list`)
installed_pip_package_check "$pip3_installed_package" "$installed"
