#!/bin/bash
## MUST: Always update this whenever a new version is released. Also remember to update download URLs if needed
APP_VERSION=2.0.0
URL_STUB="https://github.com/MatraKi/MatraKi-Keyboard/releases/download/${APP_VERSION}"

cat <<"EOF"
|-------------------------------------------------------|
|-----Welcome to the MatraKi Keyboard installer------|
|-------------------------------------------------------|

This helper script will install/update MatraKi Keyboard in your system.
You might need to type your password for the installation to proceed.

After the installation completes successfully, you would need to configure
your desktop environment for MatraKi Keyboard to work.
Visit https://github.com/MatraKi/MatraKi-Keyboard/wiki/Configuring-Environment
for instructions on how to configure your desktop environment.
--------------------------------------------------------

EOF

# detect distro
function get_distro {
  local distro_name distro_data distro_str
  # try to get distro data from *-release files
  distro_data=$(for n in /etc/*-release; do [ ! -d "$n" ]&&cat "$n"; done)
  while read -r line; do
      if [[ $line =~ ^(NAME|DISTRIB_ID)=(.+)$ ]]; then
      distro_str=${BASH_REMATCH[2]}
      if echo "$distro_str" | grep -qP '(?i).*(ubuntu|mint).*'; then
        distro_name='ubuntu'
      elif echo "$distro_str" | grep -qP '(?i).*fedora.*'; then
        distro_name='fedora'
      elif echo "$distro_str" | grep -qP '(?i)^[''"]?(arch|manjaro|antergos|chakra|magpie|parabola|anarchy).*'; then
        distro_name='arch'
      fi
      [ $distro_name ] && break
    fi
  done <<< "$distro_data"
  # plain arch installation doesn't guarantee /etc/os-release
  # but the filesystem pkg installs a blank /etc/arch-release
  if [ -z "$distro_name" ]; then
    if [ -e /etc/arch-release ]; then
      distro_name='arch'
    fi
  fi
  # TODO if os not detected, try package-manager detection (not 100% reliable)
  echo "$distro_name"
}

DISTRO_NAME=$(get_distro)
echo "Downloading installation package for $DISTRO_NAME ..."
# start downloading and install
case $DISTRO_NAME in
("ubuntu")
  # VERSION_ID (and some more vars) are sourced
  # shellcheck disable=SC1091
  source /etc/os-release
  if [ $ID = "linuxmint" ]; then
   	VERSION_ID=$(grep DISTRIB_RELEASE /etc/upstream-release/lsb-release|cut -d= -f2)
  fi
	  case ${VERSION_ID%%.*} in
	  	25|24|23|22|21|20|18) VERSION_ID=${VERSION_ID%%.*}.04;UBUNTU_SUPPORTED=1;;
	  	*) echo "This Ubuntu release \"$VERSION_ID\" is too young or too old for me to handle";exit 1;;
	  esac
  if [[ $UBUNTU_SUPPORTED = 1 ]]; then
    wget -q --show-progress "${URL_STUB}/MatraKi-Keyboard_${APP_VERSION}-ubuntu${VERSION_ID}.deb" -O "$HOME/MatraKi.deb"
    sudo apt install "$HOME/MatraKi.deb"
    mv -f "$HOME/MatraKi.deb" /tmp/
  else
    echo "Ubuntu version not supported ($VERSION_ID)."
  fi
  ;;
("arch")
  wget -q --show-progress "${URL_STUB}/MatraKi-Keyboard_${APP_VERSION}-archlinux.pkg.tar.zst" -O /tmp/MatraKi.pkg.tar.zst
  sudo pacman -U /tmp/MatraKi.pkg.tar.zst
  ;;
("fedora")
  # fedora version check ?
  wget -q  "${URL_STUB}/MatraKi-Keyboard_${APP_VERSION}-fedora38.rpm" -O /tmp/MatraKi.rpm
  sudo dnf install /tmp/MatraKi.rpm
  ;;
(*)
  echo
  echo "Sorry, this script was not able to identify your OS distribution."
  echo "Please report a bug at https://github.com/MatraKi/MatraKi-Keyboard/issues"
  echo
  echo "Please visit https://github.com/MatraKi/MatraKi-Keyboard/wiki/Installing-MatraKi-Keyboard for distrowise/distro-specific install instructions."
  echo

  echo -n "Do you want to see the instructions? (Yes/No) "
  read -r answer
  case $answer in
  [yY]*)
    URL="https://github.com/MatraKi/MatraKi-Keyboard/wiki/Installing-MatraKi-Keyboard"
    [[ -x $BROWSER ]] && exec "$BROWSER" "$URL"
    path=$(command -v xdg-open || command -v gnome-open) && exec "$path" "$URL"
    echo "Can't find browser"
    ;;
  [nN]*)
    exit
    ;;
  esac
  ;;
esac
