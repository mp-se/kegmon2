# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'Kegmon'
copyright = '2022-2026, Magnus Persson'
author = 'Magnus Persson'

# The full version, including alpha/beta/rc tags
release = '2.0.0'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
  'sphinx_copybutton'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'furo'
html_logo = "images/kegmon_logo.png"
html_title = "Kegmon 2.0.0"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_show_sourcelink = False
html_show_sphinx = True

# -- Custom 3D Model Conversion ----------------------------------------------
import os
from sphinx.util import logging

logger = logging.getLogger(__name__)

def convert_3d_models(app):
    """
    Automatically converts STL files in source/3d/ to GLB in source/_static/3d/
    during the Sphinx build process.
    """
    try:
        import trimesh
    except ImportError:
        logger.warning("trimesh library not found; skipping STL to GLB conversion. "
                       "Install it via 'pip install trimesh numpy'.")
        return

    # Paths relative to the source directory
    source_dir = os.path.join(app.srcdir, '3d')
    target_dir = os.path.join(app.srcdir, '_static', '3d')

    if not os.path.exists(source_dir):
        logger.info(f"3D source directory not found: {source_dir}")
        return

    os.makedirs(target_dir, exist_ok=True)

    for filename in os.listdir(source_dir):
        if filename.lower().endswith('.stl'):
            stl_path = os.path.join(source_dir, filename)
            glb_path = os.path.join(target_dir, os.path.splitext(filename)[0] + '.glb')
            static_stl_path = os.path.join(target_dir, filename)

            # Copy STL to static if it doesn't exist or is older
            if not os.path.exists(static_stl_path) or os.path.getmtime(stl_path) > os.path.getmtime(static_stl_path):
                import shutil
                shutil.copy2(stl_path, static_stl_path)

            # Only convert if GLB is missing or STL is newer
            if not os.path.exists(glb_path) or os.path.getmtime(stl_path) > os.path.getmtime(glb_path):
                logger.info(f"Converting 3D model: {filename} -> {os.path.basename(glb_path)}")
                try:
                    mesh = trimesh.load(stl_path)
                    # Export as GLB
                    mesh.export(glb_path)
                except Exception as e:
                    logger.error(f"Failed to convert {filename}: {e}")

def setup(app):
    # Connect the conversion function to the 'builder-inited' event
    app.connect('builder-inited', convert_3d_models)
