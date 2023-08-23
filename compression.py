#!/bin/python3.10

# https://neerc.ifmo.ru/wiki/index.php?title=%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC%D1%8B_LZ77_%D0%B8_LZ78

def lz778(data):
    prefixes = {"": 0}
    buffer = ""
    ans = []
    for i in range(len(data)):
        if ((buffer + data[i]) in prefixes.keys()):
            buffer += data[i]
        else:
            ans.append((prefixes[buffer], data[i]))
            prefixes.update({buffer + data[i]: len(prefixes)})
            buffer = ""
    if len(buffer) > 0:
        last_ch = buffer[len(buffer) - 1]
        ans.append((prefixes[buffer[:len(buffer) - 1]], last_ch))
    return ans

def compress(data):
    converted = lz778(data)
    buffer = ""
    for (i, ch) in converted:
        if i == 0:
            buffer += f",{ch}"
        else:
            buffer += f"{i},{ch}"
    return buffer
    

if __name__ == "__main__":
    data = '{"wf":{"ss":"PUK","ps":"donttouch","md":1},"dn":"Device s","cg":{"light_bright":2096,"light_open":2000,"light_close":1000,"accuracy":200,"delay":5000}}'
    # data = "abacababacabc"
    print(f"original: [{len(data)}] {data}")
    result = compress(data)
    print(f"result: [{len(result)}] {result}")
    # correct = [(0, 'a'), (0, 'b'), (1,'c'),(1,'b'),(4,'a'),(0,'c'),(4,'c')]
    # print(f"correct:[{len(correct)}] {correct}")
    # print(f"Equals? {correct == result}")
