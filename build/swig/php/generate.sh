# =================================================================
# Copyright (C) 2014 BizStation Corp All rights reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
# =================================================================
if [ $# -ge 1 ]; then
  TRANSACTD_ROOT="$(cd -- "$1"; pwd)"
else
  TRANSACTD_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE:-${(%):-%N}}")/../../../"; pwd)"
fi

OUTPUT_ROOT="${TRANSACTD_ROOT}/build/swig/php/gen"
SWIG_I_FILE="${TRANSACTD_ROOT}/build/swig/tdcl.i"
SWIG_OUTFILE="${OUTPUT_ROOT}/tdclphp_wrap.cpp"
SWIG_TDCLPHP="${OUTPUT_ROOT}/transactd.php"
SWIG_LOGFILE="${OUTPUT_ROOT}/generate.log"
SWIG_ERRORLOG="${OUTPUT_ROOT}/generate.err.log"

if [ ! -e "${OUTPUT_ROOT}" ]; then
  mkdir "${OUTPUT_ROOT}"
fi
if [ -e "${SWIG_TDCLPHP}.correct" ]; then
  rm "${SWIG_TDCLPHP}.correct"
fi

pushd "${TRANSACTD_ROOT}" > /dev/null

# Generate correct transactd.php
# see SWIG bugs:#1350 http://sourceforge.net/p/swig/bugs/1350/
cp "${SWIG_I_FILE}" "${SWIG_I_FILE}.org"
grep -v "^%newobj" "${SWIG_I_FILE}.org" > "${SWIG_I_FILE}"
swig -c++ -php5 -I"${TRANSACTD_ROOT}" -I"${TRANSACTD_ROOT}/source" \
  -o "${SWIG_OUTFILE}" "${SWIG_I_FILE}" > "${SWIG_LOGFILE}" 2> "${SWIG_ERRORLOG}"
rm "${SWIG_I_FILE}"
mv "${SWIG_I_FILE}.org" "${SWIG_I_FILE}"
mv "${SWIG_TDCLPHP}" "${SWIG_TDCLPHP}.correct"

# Generate correct tdclphp_wrap.cpp php_transactd.h
swig -c++ -php5 -I"${TRANSACTD_ROOT}" -I"${TRANSACTD_ROOT}/source" \
  -o "${SWIG_OUTFILE}" "${SWIG_I_FILE}" >> "${SWIG_LOGFILE}" 2>> "${SWIG_ERRORLOG}"
rm "${SWIG_TDCLPHP}"
mv "${SWIG_TDCLPHP}.correct" "${SWIG_TDCLPHP}"

popd > /dev/null
