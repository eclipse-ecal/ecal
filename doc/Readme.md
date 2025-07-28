# Eclipse eCAL™ Documentation

This documentation is created with Sphinx. Sphinx uses the reStructuredText (RST) markup format and adds some features like cross-referencing.

To create the documentation, it's necessary to have the requirements from `requirements.txt` installed.
They have been locked from `pyproject.toml` using pdm.

```
pip install pdm 
pdm lock --python=">=3.11"
pdm lock --python="<3.11" --append
pdm plugin add export
pdm export -f requirements --without-hashes > requirements.txt
```

This way, newer Python version can use newer Sphinx versions.

