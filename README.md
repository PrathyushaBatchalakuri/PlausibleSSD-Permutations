
# Plausibly Deniable SSD
## _Secure Data Storage with Data Layout Permutation_

This project implements a novel approach to enhance data privacy in Solid State Drives (SSDs) by enabling plausible deniability through data layout permutation. Our system allows users to convincingly deny the existence of sensitive data, thus protecting against both digital and physical threats.

- Implement data permutation in SSDs
- Provide plausible deniability
- ✨Magic ✨

## Features

- **Plausible Deniability**: Users can convincingly deny the existence of sensitive data.
- **Data Layout Permutation**: Data blocks are rearranged in a non-linear pattern to secure data.
- **Multithreading**: Utilize modern CPUs' concurrent processing capabilities to speed up file handling.

## Installation and Setup

Ensure you have Homebrew installed on your macOS system for managing software packages. If not, install Homebrew and the necessary dependencies, compile the source files, and execute the program using the following commands:


# Install Homebrew
To install Homebrew from its official repository, configuring it based on your OS version for optimal performance.
```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

# Install dependencies
With Homebrew installed, use it to install required software packages such as GMP for arithmetic operations, LibreOffice for document handling, and Poppler for PDF processing using straightforward `brew install` commands.
```sh
brew install gmp # GMP for high precision arithmetic operations
brew install --cask libreoffice # LibreOffice for handling DOCX files
brew install poppler # Poppler for handling PDF files
```

# Compile the source files
- Use the GCC command to compile the source files `ranking_unranking.c` and `thpool.c` with threading support enabled (`-pthread`). 

- The command specifies output to `outputFile`. 

- It includes directories for the GMP library headers (`-I/opt/homebrew/opt/gmp/include`) and links against the GMP library for advanced arithmetic operations (`-L/opt/homebrew/opt/gmp/lib -lgmp`). 
- This process results in an executable named `outputFile` ready for execution.
```sh
gcc -pthread -o outputFile ranking_unranking.c thpool.c -I/opt/homebrew/opt/gmp/include -L/opt/homebrew/opt/gmp/lib -lgmp
```

# Execute the program
Run the `outputFile` executable created by the GCC compilation. Pass `textfile.txt`, `wordfile.docx`, and `pdffile.pdf` as command-line arguments to the program.
```sh
./outputFile textfile.txt wordfile.docx pdffile.pdf
```


## Experimental Results

- **Data Integrity Maintained**: Successfully processed `.txt`, `.docx`, and `.pdf` files without data loss.
- **Performance Efficiency**: Demonstrated minimal latency in key functions:
  - **Load Text to GMP**: Approximately 0.003 seconds.
  - **Rank Calculation**: Approximately 0.000023 seconds.
  - **Permutation Generation**: Approximately 0.000098 seconds.
- **Total Execution Time**: Completed all operations in about 5.655 seconds.
- **Security and Efficiency**: The permutation techniques enhanced SSD security effectively, proving robust against digital and physical threats while maintaining operational performance.
