# Checking again to ensure that both lists have 10000 addresses.
def verify_address_count(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    # Extract 8-digit hexadecimal addresses and remove last 4 digits
    hex_addresses = [line.strip() for line in lines if len(line.strip()) > 4 and line.strip() != '10000']
    # processed_addresses = [addr[:4] for addr in hex_addresses]  # Keeping only the first 4 digits
    # remove last 4 digits
    processed_addresses = [addr[:-4] for addr in hex_addresses]
    # convert to int from hex
    processed_addresses = [str(int(addr, 16)) for addr in processed_addresses]

    # Verifying if two lists of exactly 10000 each can be made
    list1 = processed_addresses[:10000]
    list2 = processed_addresses[10000:20000]
    print(len(processed_addresses))
    for num in list2:
        if(len(num) < 4):
            print(num)
    #print list2 to a file
    with open('dbg2.txt', 'w') as file:
        for addr in list2:
            file.write(addr + ',')

    return len(list1), len(list2)

a = verify_address_count('dbg.txt') 
print(a)