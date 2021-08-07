import os
from pathlib import Path

PATH_VAR_ROOT = Path(os.path.abspath(os.curdir))
ROOT_DIR = PATH_VAR_ROOT.absolute()
OBSW_ROOT_DIR = PATH_VAR_ROOT.parent.absolute()
DATABASE_NAME = "source_mod.db"
