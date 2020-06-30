import setuptools

with open("README.md" ,"r") as fh:
	long_description = fh.read()

setuptools.setup(
	name = "loauth",
	version = "0.0.1",
	author = "thebox",
	author_email = "ahaandabholkar@gmail.com",
	description = "Authentication Library",
	long_description = long_description,
	long_description_content_type = "text/markdown",
	url = "https://github.com/theboxahaan/loauth",
	packages = setuptools.find_packages(),
	python_requires = '>=3.6',
)


