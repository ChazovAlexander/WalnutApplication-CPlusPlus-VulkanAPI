with open('7.png', 'rb') as file:
    png_data = file.read()

# Convert the binary data to a list of hexadecimal values
hex_values = ', '.join([f'0x{byte:02x}' for byte in png_data])

with open('1.txt', 'wb') as file:
    # Write the hexadecimal values to the text file
    file.write(hex_values.encode('utf-8'))