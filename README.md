# Distributed Botnet
[![ForTheBadge built-with-swag](http://ForTheBadge.com/images/badges/built-with-swag.svg)](https://GitHub.com/Naereen/)
![made-with-python](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![boost](https://img.shields.io/badge/Bootstrap-563D7C?style=for-the-badge&logo=bootstrap&logoColor=white)
[![MIT License][license-shield]][license-url]
![btc](https://img.shields.io/badge/Bitcoin-000000?style=for-the-badge&logo=bitcoin&logoColor=white)
![porsche](https://aleen42.github.io/badges/src/porsche.svg)




<!-- CONTACT -->
## Contact

- Markiyan Valyavka - [github](https://github.com/markvalyavka) - valyavka@ucu.edu.ua
- Andrew Bek - [github](https://github.com/ReyBroncas) - bek@ucu.edu.ua
- Olexandra Hutor - [github](https://github.com/Oleksandra2020) - hutor@ucu.edu.ua

Project Link: [https://github.com/Oleksandra2020/botnet](https://github.com/Oleksandra2020/botnet)


<!-- TABLE OF CONTENTS -->
## Table of Contents

* [About the Project](#about-the-project)
* [Installation](#installation)
* [Contributing](#contributing)
* [Usage examples](#usage)
* [License](#license)
* [Contact](#contact)


<!-- ABOUT THE PROJECT -->
## About The Project

The botnet is a network of computers that has a server-computer with clients, who get the victims and commands from the server to perform an attack on the victim.



<!-- INSTALLATION -->
## Installation

1. Clone the repo
```sh
git clone git@github.com:Oleksandra2020/botnet.git
```
2. Build & compile

```bash
./scripts/build.sh
```
or 

```bash
cmake --build build -DCMAKE_BUILD_TYPE=Release && cd build && make
```
3. Run the server
```bash
./build/botnet server <local-port-to-run-on>
```
3. Run the client
```bash
sudo ./build/botnet client <server-public-ip> <server-public-port> <local-port-to-run-on>
```
3. Run the manager
```bash
./build/botnet admin <server-public-ip> <server-public-port> <local-port-to-run-on>
```






<!-- USAGE EXAMPLES -->
## Usage

1. Run as specified above
2. Using the instructions for admin user add/remove clients and victims

### Example
![alt text][example]


<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to be learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request



<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE` for more information.








<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[example]: https://github.com/Oleksandra2020/botnet/blob/master/res/example.gif

[made-with-python-shield]: https://img.shields.io/badge/Made%20with-Python-1f425f.svg
[made-with-python-url]: https://www.python.org/
[contributors-shield]: https://img.shields.io/github/contributors/Naereen/StrapDown.js.svg
[contributors-url]: https://github.com/markvalyavka/cheaply_route_app/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/othneildrew/Best-README-Template.svg?style=flat-square
[forks-url]: https://github.com/othneildrew/Best-README-Template/network/members
[stars-shield]: https://img.shields.io/github/stars/othneildrew/Best-README-Template.svg?style=flat-square
[stars-url]: https://github.com/othneildrew/Best-README-Template/stargazers
[issues-shield]: https://img.shields.io/github/issues/Naereen/StrapDown.js.svg
[issues-url]: https://github.com/markvalyavka/cheaply_route_app/issues
[license-shield]: https://img.shields.io/badge/License-MIT-blue.svg
[license-url]: https://github.com/markvalyavka/cheaply_route_app/blob/master/LICENSE
[product-screenshot]: images/screenshot.png
