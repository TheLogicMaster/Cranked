#!/usr/bin/python3

# Script to convert resources to C++ header friendly formats

import argparse


def embed_binary(file, var_name, header_name):
    with open(file, 'rb') as f, open(header_name, 'w') as output:
        data = f.read()
        output_text = '// Autogenerated file (Do not edit)\n\n'
        output_text += '#pragma once\n\n'
        output_text += f'constexpr unsigned char {var_name}[] {{'
        for i in range(len(data)):
            if i % 16 == 0:
                output_text += '\n\t'
            output_text += f'{hex(data[i])}, '
        output_text += '\n};\n'
        output.write(output_text)


def embed_text(file, var_name, header_name):
    with open(file) as f, open(header_name, 'w') as output:
        text = f.read()
        output_text = '// Autogenerated file (Do not edit)\n\n'
        output_text += '#pragma once\n\n'
        output_text += f'constexpr char {var_name}[] =\n\t"'
        i = 0
        while i < len(text):
            char = text[i]
            if char == '"':
                output_text += '\\"'
            elif char == '\r' or char == '\n':
                if char == '\r':
                    i += 1
                output_text += '\\n"\n\t"'
            elif char == '\\':
                output_text += r'\\'
            else:
                output_text += char
            i += 1
        output_text += '";\n'
        output.write(output_text)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='embed_resource', description='Converts text and binary files to C++ header files')
    parser.add_argument('file')
    parser.add_argument('header')
    parser.add_argument('var')
    parser.add_argument('-t', '--text', action='store_true')
    args = parser.parse_args()
    if args.text:
        embed_text(args.file, args.var, args.header)
    else:
        embed_binary(args.file, args.var, args.header)