# loauth - Authentication Protocol Library
Initial documentation to help people get started. Setting up a `virtualenv` is highly recommended for the development of this library.

## Extending the `protopack` 
Currently `loauth` only has support for a rudimentary *challenge-response* protocol based on `SHA256` HMACs. Other modules for other protocols should be developed similar to this module. Each protocol should have a single `client` and `butler` function. 
**Guidelines** Modules can be tested on the provided `devbutler` module.

### Development
> Install Library using `pip` in **development mode** using

> ```bash (venv)$ pip install -e .```

### Setting Up The `devbutler` Development Server

The setup and use of the development server is shown below with the CHAP protocol usecase -
```python
>>> from loauth.devbutler import deploy_butler
>>> from loauth.protopack.chap import CHAP_Butler
>>> deploy_butler(CHAP_Butler, 4000)
```
This well setup the server to listen on `localhost:4000`. The `CHAP_Client` test can be initialised as shown
```bash
$ python chap.py
```
*Refer to the implementation for further details*
___
