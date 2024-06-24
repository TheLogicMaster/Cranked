#!/usr/bin/python

# Ingests C headers from $ENV{PLAYDATE_SDK_PATH} and populates the headers for this project

import re
import os
import sys


FUNC_PTR_PATTERN = r'\s*(.+?)\s*\(\s*\*\s*(\w+)\s*\)\s*\((.*?)\)\s*'
FIELD_PATTERN = r'\s*(.+?)((?<!\w)\w+)\s*(\[.*?])?\s*(:\s*\d+)?\s*'
PARAM_PATTERN = r'\s*(.+?)((?<!\w)\w+)?\s*(\[.*?])?\s*(:\s*\d+)?\s*'
COMMA_SPLIT_MAGIC = r',\s*(?![^()]*\))'  # https://stackoverflow.com/a/26634150

KNOWN_STRUCTS = ['SDFile']
CUSTOM_TYPES = [
    ('SDFile', 'uint32_t', 'SDFile_32'),
    ('SpriteCollisionResponseType', 'int8_t', 'int8_t'),
]
CUSTOM_NATIVE_TYPES = {
    'LCDRect': 'ArgType::struct4_t',
    'PDRect': 'ArgType::struct4f_t',
    'json_value': 'ArgType::struct2_t',
    'json_reader': 'ArgType::struct2_t',
}
CUSTOM_API_STRUCTS = ['json_encoder']
NULL_STRUCT_FIELDS = {
    'playdate_graphics': ['getDebugBitmap']
}


class CType:
    def __init__(self, c_type: str, emu_type=None, wrapper_type=None, array=''):
        self.c_type = c_type.strip()
        if emu_type is not None:
            self.emu_type = emu_type
            self.wrapper_type = wrapper_type
            return
        ptr = array.count('[')
        c_type = re.sub(r'(?:(?<!\w)const\s)+', '', c_type, re.MULTILINE)
        c_type = c_type.strip()
        if match := re.match(r'\s*struct (\w+)\s*\*', c_type):
            self.emu_type = 'uint32_t'
            self.wrapper_type = match[1] + '_32 ' + '*' * c_type.count('*') if c_type.count('*') <= 1 else 'cref_t *'
            return
        if re.match(r'\s*(?<!\w)(?:lua_CFunction|PD\w+CallbackFunction)\s*', c_type):
            self.emu_type = 'uint32_t'
            self.wrapper_type = 'cref_t'
            return
        if '*' in c_type:
            ptr += c_type.count('*')
            c_type = c_type.replace('*', '')
        if re.match(r'\s*void\s*', c_type):
            self.emu_type = 'void' if ptr == 0 else 'uint32_t'
            self.wrapper_type = 'void' if ptr == 0 else 'void ' + '*' * ptr if ptr == 1 else 'cref_t *'
            return
        if re.match(r'\s*\.\.\.\s*', c_type):
            self.emu_type = '...'
            self.wrapper_type = '...'
            return
        if re.match(r'\s*va_list\s*', c_type):
            self.emu_type = 'va_list'
            self.wrapper_type = '...'
            return
        type_replacements = {
            r'unsigned\s+long\s+long': 'uint64_t',
            r'(signed)?\s*long\s+long': 'int64_t',
            r'unsigned\s+(long)?\s*(?:int|long)': 'uint32_t',
            r'(signed)?\s*(long)?\s*(?:int|long)': 'int32_t',
            r'unsigned\s+short': 'uint16_t',
            r'(signed)?\s*short': 'int16_t',
            r'signed\s+char': 'int8_t',
            r'(unsigned)?\s*char': 'uint8_t',
            'float': 'float',
            'double': 'double',
            'size_t': 'uint32_t',
            'uintptr_t': 'uint32_t',
            'intptr_t': 'int32_t',
        }

        matched = re.match(r'u??int\d+_t', c_type) is not None
        if not matched:
            for value, replacement in type_replacements.items():
                if re.match(rf'\s*{value}\s*', c_type):
                    c_type = replacement
                    matched = True
                    break
        if not matched:
            raise Exception(f'Failed to parse type for: `{c_type}`')
        if ptr == 0:
            self.emu_type = c_type
            self.wrapper_type = c_type
        else:
            self.emu_type = 'uint32_t'
            self.wrapper_type = c_type + ' ' + '*' * ptr if ptr == 1 else 'cref_t *'

    def __str__(self):
        return f'{self.emu_type}'


class CField:
    def __init__(self, name: str, c_type: CType, width=0):
        self.field_name = name.strip() if name else None
        self.c_type = c_type
        self.width = width

    def __str__(self):
        struct_type = 'cref_t' if self.c_type.wrapper_type == 'cref_t' or '*' in self.c_type.wrapper_type else self.c_type
        if self.field_name is None:
            return f'{struct_type}'
        else:
            return f'{struct_type} {self.field_name}' if self.width == 0 else f'{struct_type} {self.field_name} : {self.width}'


class CUnion:
    def __init__(self, name: str):
        self.name = name.strip()
        self.fields: list[CField] = []

    def __str__(self):
        field_str = "".join([f"\t\t{field};\n" for field in self.fields])
        return f'union {{\n{field_str}\t}} {self.name}'


class CFuncPtr:
    def __init__(self, name: str, return_type: CType, owner: 'Struct|None'):
        self.name = name.strip()
        self.qualified_name = f'{owner.struct_name}_{name}' if owner else name
        self.return_type = return_type
        self.params: list[CField | CFuncPtr | CType] = []
        self.owner: Struct | None = owner
        self.index = -1

    def __str__(self):
        return f'cref_t {self.name}'


class Struct:
    def __init__(self, name: str):
        self.struct_name = name.strip()
        self.fields: list[CField | CUnion | CFuncPtr] = []
        self.parent_api = None
        self.field_name = ''

    def __str__(self):
        fields_str = "".join([f"\t{field};\n" for field in self.fields])
        return f'struct {self.struct_name}_32 {{\n{fields_str}}};\n'


class VersionedFunction:
    def __init__(self, until: str, name: str, return_type: CType, params: list[CField | CFuncPtr | CType]):
        """ Until version is exclusive, name is replacement name """
        self.name = name
        self.until = until
        self.func = CFuncPtr(name, return_type, None)
        self.func.params = params


# Versions must be listed in increasing order for a given API function
VERSIONED_API_FUNCTIONS = {
    'playdate_sound_sequence_setTempo': [
        VersionedFunction('2.1.0', 'playdate_sound_sequence_setTempo_int', CType('void'), [
            CField('sequence', CType('struct SoundSequence *')),
            CField('stepsPerSecond', CType('int')),
        ]),
    ],
}


def main():
    sdk_path = sys.argv[1] if len(sys.argv) > 1 else os.getenv('PLAYDATE_SDK_PATH')
    if not sdk_path:
        raise Exception('PLAYDATE_SDK_PATH not set or passed in as an argument')

    project_path = os.path.join(os.path.realpath(os.path.dirname(__file__)), os.pardir)
    gen_path = os.path.join(project_path, 'core', 'gen')
    base_path = os.path.join(sdk_path, 'C_API')
    subdir = os.path.join(base_path, 'pd_api')
    files = [os.path.join(base_path, 'pd_api.h')]
    for name in os.listdir(subdir):
        path = os.path.join(subdir, name)
        if os.path.isfile(path):
            files.append(path)

    with open(os.path.join(sdk_path, 'VERSION.txt'), 'r') as f:
        sdk_version = f.readline().rstrip()

    def find_import_index(n: str):
        ordering = ['gfx', 'sys', 'lua', 'json', 'file', 'sprite', 'sound', 'display', 'scoreboards']
        for i in range(len(ordering)):
            if ordering[i] in n:
                return i
        return len(ordering) + 1
    files.sort(key=find_import_index)

    known_structs = set(KNOWN_STRUCTS)
    known_enums = set()
    known_function_types = set()
    known_typedefs: dict[str, str] = dict()
    structs: list[Struct] = []
    for path in files:
        with open(path) as file:
            contents = file.read()
        contents = '\n'.join([line.split('//')[0] for line in contents.splitlines()])
        contents = re.sub(r'union\s*\{[\w\W]+?}', lambda m: m[0][:-1] + '|', contents, re.MULTILINE)

        for match in re.findall(r'^\s*(?:typedef)??\s*enum\s*\{[\w\W]*?}\s*(\w+)\s*;', contents, re.MULTILINE):
            known_enums.add(match)
        for match in re.findall(r'^\s*typedef\s+\w+\s*\**\s*\(?\*?(\w+)\)?\(.*?\)\s*;', contents, re.MULTILINE):
            known_function_types.add(match)
        for match in re.findall(r'\s*typedef\s*struct\s*\w+\s*(\w+)\s*;', contents):
            known_structs.add(match)
        for match in re.findall(r'\s*typedef\s+(\w+)\s*(\w+)\s*(\[\d+])?\s*;', contents):
            known_typedefs[match[1]] = match[0] + '*' * match[2].count('[')

        struct_matches = re.findall(r'^\w*\s*struct\s*(\w*)\s*\{([\w\W]+?)}\s*(\w*)\s*;', contents, re.MULTILINE)
        struct_matches = [(match[0], match[1]) if match[0] else (match[2], match[1]) for match in struct_matches]
        for struct in [match[0] for match in struct_matches]:
            known_structs.add(struct)

        def get_type(c_type: str) -> CType:
            if re.match(r'\s*(?:const)?\s*PD\w*CallbackFunction\s*\*\s*', c_type):
                return CType(c_type, emu_type='uint32_t', wrapper_type='cref_t')
            for custom in CUSTOM_TYPES:
                if re.fullmatch(rf'\s*(?:const)?\s*{custom[0]}\s*\**\s*', c_type):
                    ptr_count = c_type.count('*')
                    if ptr_count == 1:
                        return CType(custom[0], emu_type='uint32_t', wrapper_type=custom[2] if custom[2] == 'cref_t' else f'{custom[2]} ' + '*' * ptr_count)
                    elif ptr_count > 1:
                        return CType(custom[0], emu_type='uint32_t', wrapper_type='cref_t *')
                    return CType(custom[0], emu_type=custom[1], wrapper_type=custom[2])
            for typedef, replacement in known_typedefs.items():
                c_type = re.sub(rf'(?<!\w){typedef}(?!\w)', replacement, c_type)
            for func_type in known_function_types:
                if re.fullmatch(rf'\s*(?:const)?\s*{func_type}\s*\**\s*', c_type):
                    return CType(c_type, emu_type='uint32_t', wrapper_type='cref_t')
            if re.fullmatch(rf'\s*(?:const)?\s*enum\s+\w+\s*', c_type):
                return CType(c_type, emu_type='int32_t', wrapper_type='int32_t')
            for enum in known_enums:
                if re.fullmatch(rf'\s*(?:const)?\s*{enum}\s*\*+\s*', c_type):
                    return CType(c_type, emu_type='uint32_t', wrapper_type='int32_t ' + '*' * c_type.count('*') if c_type.count('*') <= 1 else 'cref_t *')
                elif re.fullmatch(rf'\s*(?:const)?\s*{enum}\s*', c_type):
                    return CType(c_type, emu_type='int32_t', wrapper_type='int32_t')
            for s in known_structs:
                if re.fullmatch(rf'\s*(?:const)?\s*{s}\s*\*+\s*', c_type):
                    return CType(c_type, emu_type='uint32_t', wrapper_type=f'{s}_32 ' + '*' * c_type.count('*') if c_type.count('*') <= 1 else 'cref_t *')
                elif re.fullmatch(rf'\s*(?:const)?\s*{s}\s*', c_type):
                    return CType(c_type, emu_type=f'{s}_32', wrapper_type=f'{s}_32')
            return CType(c_type)

        for struct_match in struct_matches:
            struct = Struct(struct_match[0])
            structs.append(struct)
            content = '\n'.join([line.split('//')[0].strip() for line in struct_match[1].splitlines()])
            lines = []
            split_depth = 0
            current_line = ''
            for char in content:
                current_line += char
                if char == '{':
                    split_depth += 1
                elif char == '|':
                    split_depth -= 1
                elif char == ';' and split_depth == 0:
                    lines.append(current_line)
                    current_line = ''
            for line in lines:
                if match := re.match(rf'{FUNC_PTR_PATTERN};', line, re.MULTILINE):
                    def parse_func_ptr_args(code: str) -> list[CFuncPtr | CField]:
                        args = []
                        if not re.fullmatch(r'\s*void\s*', code):
                            for arg in re.split(COMMA_SPLIT_MAGIC, code):
                                if re.match(r'\s*\.\.\.\s*', arg):
                                    args.append(CField('', get_type('...')))
                                elif ptr_match := re.match(rf'{FUNC_PTR_PATTERN}$', arg):
                                    args.append(CField(ptr_match[2], CType(arg, emu_type='uint32_t', wrapper_type='cref_t')))
                                elif ptr_match := re.match(rf'{PARAM_PATTERN}$', arg):
                                    args.append(CField(ptr_match[2], get_type(ptr_match[1])))
                                else:
                                    raise Exception(f'Failed to parse function pointer arg: `{arg}`')
                        return args
                    ptr = CFuncPtr(match[2], get_type(match[1]), struct)
                    struct.fields.append(ptr)
                    ptr.params += parse_func_ptr_args(match[3])
                elif match := re.match(rf'{FIELD_PATTERN};', line):
                    width = int(match[4][1:]) if match[4] else 0
                    struct.fields.append(CField(match[2], get_type(match[1]), width))
                elif match := re.match(r'\s*union\s*\{([\w\W]*?)\|\s*(\w+)\s*;', line, re.MULTILINE):
                    union = CUnion(match[2])
                    struct.fields.append(union)
                    for union_line in match[1].split(';'):
                        union_line = union_line.strip()
                        if not union_line:
                            continue
                        union_line = union_line + ';'
                        if union_match := re.match(rf'{FIELD_PATTERN};', union_line, re.MULTILINE):
                            union.fields.append(CField(union_match[2], get_type(union_match[1])))
                        else:
                            raise Exception(f'Failed to parse union field: `{union_line}`')
                else:
                    raise Exception(f'Failed to parse struct field: `{line}`')

    def find_struct(struct_name: str):
        for s in structs:
            if s.struct_name == struct_name:
                return s
        raise Exception(f'Failed to find struct for: `{struct_name}`')

    def collect_api_structs(api_struct: Struct, parent: Struct | None, field_name: str):
        nonlocal function_index
        if parent:
            api_struct.parent_api = parent
            api_struct.field_name = field_name
        api_structs.append(api_struct)
        for struct_field in api_struct.fields:
            if type(struct_field) is CField and 'struct' in struct_field.c_type.c_type:
                collect_api_structs(find_struct(struct_field.c_type.wrapper_type.split('_32')[0]), api_struct, struct_field.field_name)
            elif type(struct_field) is CFuncPtr and (api_struct.struct_name not in NULL_STRUCT_FIELDS or struct_field.name not in NULL_STRUCT_FIELDS[api_struct.struct_name]):
                struct_field.index = function_index
                functions.append(struct_field)
                function_index += 1
                if struct_field.qualified_name in VERSIONED_API_FUNCTIONS:
                    for versioned in VERSIONED_API_FUNCTIONS[struct_field.qualified_name]:
                        versioned.func.index = function_index
                        functions.append(versioned.func)
                        function_index += 1
    api_structs = []
    functions: list[CFuncPtr] = []
    function_index = 0
    collect_api_structs(find_struct('PlaydateAPI'), None, '')

    for custom_api_struct in CUSTOM_API_STRUCTS:
        collect_api_structs(find_struct(custom_api_struct), None, '')

    with open(os.path.join(gen_path, 'PlaydateAPI.hpp'), 'w') as f:
        f.write('// Auto-generated Playdate compatible struct definitions (32-bit word size)\n\n')

        f.write('#pragma once\n\n')

        f.write('#include "../PlaydateTypes.hpp"\n')
        f.write('#include "Utils.hpp"\n\n')

        f.write(f'#define PLAYDATE_SDK_VERSION "{sdk_version}"\n\n')

        f.write(f'constexpr int FUNCTION_TABLE_SIZE = {len(functions)};\n\n')

        f.write('namespace cranked {\n\n')

        f.write('class Cranked;\n\n')

        f.write('extern NativeFunctionMetadata playdateFunctionTable[FUNCTION_TABLE_SIZE];\n\n')

        f.write('int populatePlaydateApiStruct(void *api, Version version);\n\n')

        for struct in structs:
            f.write(f'{struct}\n')
        f.write('\n')

        for func in functions:
            f.write(f'{func.return_type.wrapper_type} {func.qualified_name}(Cranked *cranked')
            for param in func.params:
                f.write(f', {param.c_type.wrapper_type} {param.field_name if param.field_name and param.c_type.wrapper_type != "..." else ""}')
            f.write(');\n')
        f.write('\n')

        f.write('}\n')

    with open(os.path.join(gen_path, 'PlaydateFunctionMappings.cpp'), 'w') as f:
        f.write('// Auto-generated Playdate native function mappings\n\n')
        f.write('#include "PlaydateAPI.hpp"\n')
        f.write('#include "../Cranked.hpp"\n\n')
        f.write('using namespace cranked;\n\n')

        def get_native_type(c_type: CType):
            if '*' in c_type.wrapper_type:
                return 'ArgType::ptr_t'
            elif c_type.wrapper_type == 'void':
                return 'ArgType::void_t'
            elif c_type.emu_type == 'va_list':
                return f'ArgType::va_list_t'
            elif c_type.wrapper_type == '...':
                return f'ArgType::varargs_t'
            elif re.fullmatch(r'u?int\d+_t|float|double', c_type.emu_type):
                return f'ArgType::{c_type.emu_type}{"" if c_type.emu_type.endswith("_t") else "_t"}'
            for typename, native_type in CUSTOM_NATIVE_TYPES.items():
                if typename == c_type.c_type:
                    return native_type
            raise Exception(f'Unknown native type conversion for: `{c_type.c_type}`')

        f.write('NativeFunctionMetadata cranked::playdateFunctionTable[FUNCTION_TABLE_SIZE] {\n')
        for func in functions:
            param_types = [get_native_type(param.c_type) for param in func.params]
            f.write(f'\t{{ "{func.qualified_name}", (void *) {func.qualified_name}, {{ {", ".join(param_types)} }}, {get_native_type(func.return_type)} }},\n')
        f.write('};\n\n')

        # Todo: Padding for new API functions

        f.write('int cranked::populatePlaydateApiStruct(void *api, Version version) {\n')
        f.write('\tint offset = 0;\n')
        for struct in api_structs:
            f.write(f'\tauto {struct.struct_name} = ({struct.struct_name}_32 *) ((uintptr_t) api + offset);\n')
            if struct.parent_api:
                f.write(f'\t{struct.parent_api.struct_name}->{struct.field_name} = API_ADDRESS + offset;\n')
            f.write(f'\toffset += sizeof({struct.struct_name}_32);\n')
            for field in struct.fields:
                if type(field) is CFuncPtr:
                    if field.index >= 0:
                        f.write(f'\t{struct.struct_name}->{field.name} = FUNC_TABLE_ADDRESS + {field.index} * 2 + 1;\n')
                        if field.qualified_name in VERSIONED_API_FUNCTIONS:
                            versioned_index = 0
                            for versioned in VERSIONED_API_FUNCTIONS[field.qualified_name]:
                                f.write('\t')
                                if versioned_index > 0:
                                    f.write('else ')
                                f.write(f'if (version < Version("{versioned.until}"))\n')
                                f.write(f'\t\t{struct.struct_name}->{field.name} = FUNC_TABLE_ADDRESS + {field.index + 1 + versioned_index} * 2 + 1;\n')
                                versioned_index += 1
        f.write('\treturn offset;\n')
        f.write('}\n')


if __name__ == '__main__':
    main()
