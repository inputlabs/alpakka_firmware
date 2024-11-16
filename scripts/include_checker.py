import subprocess
import glob

TEMP_FIXES_PATH = 'build/iwyu.c'

def check_file(path):
	headers = get_headers()
	command = f'include-what-you-use {path} '
	command += f'-w -include {TEMP_FIXES_PATH} {headers}'
	result = subprocess.run(command, shell=True, capture_output=True, text=True)
	notes = result.stderr
	if 'has correct #includes/fwd-decls' in notes:
		return
	if 'The full include-list' in notes:
		notes = notes.split('The full include-list')[0]
	if 'should remove these lines:' in notes:
		notes = (notes
			.strip('\n')
			.split('should remove these lines:')[1]
			.split('\n')
		)
		notes = [
			f'{path}: Unused {x.strip("- ").split("//")[0]}'
			for x in notes
			if '#include' in x
		]
		notes = '\n'.join(notes)
	print(notes.strip('\n'))

def get_headers():
	headers = [
		'src/headers',
		'build/generated/pico_base',
		'deps/pico-sdk/lib/tinyusb/src',
	]
	command = 'find deps -type d -name "include"'
	result = subprocess.run(command, shell=True, capture_output=True, text=True)
	found_headers = result.stdout.strip('\n').split('\n')
	headers += found_headers
	headers = [f'-I{x}' for x in headers]
	headers = ' '.join(headers)
	return headers

def get_sources():
	return sorted(
		glob.glob('src/*.c') +
		glob.glob('src/profiles/*.c')
	)

def generate_temp_fixes():
	with open(TEMP_FIXES_PATH, "w") as file:
		file.write('#include <stdbool.h>\n')
		file.write('#include <stdint.h>\n')
		file.write('#define __force_inline')

if __name__ == '__main__':
	generate_temp_fixes()
	sources = get_sources()
	for path in sources:
		# print('=' * 80)
		# print(path)
		check_file(path)
