@echo off

pushd %~dp0\..

call build_win\win_set_vars.bat

if not exist "%BUILD_DIR_COMPLETE%" mkdir "%BUILD_DIR_COMPLETE%"

echo Creating Python venv
if not exist "%BUILD_DIR_COMPLETE%\.venv" mkdir "%BUILD_DIR_COMPLETE%\.venv"
python -m venv "%BUILD_DIR_COMPLETE%\.venv"
CALL "%BUILD_DIR_COMPLETE%\.venv\Scripts\activate.bat"

echo Upgrading pip
python -m pip install --upgrade pip

echo Installing python requirements
pip install wheel
pip install -r doc\requirements.txt
python -m pip install build

cd /d "%WORKSPACE%\%BUILD_DIR_COMPLETE%"

python -m build ..\.. --outdir .\_deploy

popd
