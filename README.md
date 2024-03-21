# EmmmCpp
A simple approach to flexibly select and retrieve data from JSON like 'Eeny-Meeny-Miny-Moe'

## Origin of the Name
The name "Emmm" is inspired by the classic children's game of selection, "Eeny, meeny, miny, moe." This game symbolizes the process of making a random choice from multiple options, all while enjoying the selection process.  
Similarly, this library is designed to provide a variety of choices from JSON data, allowing for easy selection and retrieval of values.

## Features
- Retrieve and convert values from JSON data.
- Supports various data types (integers, floating-point, custom types).
- Concise and readable API.

## Requirements
- C++11
- [nlohmann/json](https://github.com/nlohmann/json)

## Installation
This library is a header-only library, so no installation is required.  
Simply copy the include directory into your project and add it to your include path.

```cpp
#include "EmmmCore.h"
```

## Usage

Below is a simple example of using Emmm library to select and retrieve data from a JSON object.  
In this example, we obtain values from a complex JSON structure with multiple options and their respective weights.

```cpp
#include "EmmmCore.h"
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
	// Example JSON data
	nlohmann::json json_data = R"(
    {
		"choice": [
			{
				"weight": 5,
				"const": 0
			},
			{
				"weight": 3,
				"choice": [
					{
						"weight": 7,
						"range": [0,1]
					},
					{
						"weight": 3,
						"const": 1
					}
				]
			},
			{
				"weight": 2,
				"choice": [1,2,3]
			}
		]
	}
	)" _json;

	// Create an EmmmValue object based on the JSON data
	auto value = emmm::create<float>(json_data);

	// Retrieve and print the value
	std::cout << "Retrieved value: " << value->getValue() << std::endl;

	return 0;
}
```

## License
This library is released under the MIT License.
