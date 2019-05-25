import sys

FROM = sys.argv[1]
TO = sys.argv[2]

with open(TO, 'w', encoding='big5hkscs') as T:
    z2c_dict = {}

    with open(FROM, 'r', encoding='big5hkscs') as f:
        for row in f:
            [word, zhuyin] = row.split(' ')

            for phoneme in zhuyin.split('/'):
                head = phoneme[0]
                if z2c_dict.get(head, None) is None:
                    z2c_dict[head] = [word]

                if z2c_dict[head][-1] != word:
                    z2c_dict[head].append(word)

            T.write(f'{word}\t{word}\n')

    for key in z2c_dict:
        s = ' '.join(z2c_dict[key])
        T.write(f'{key}\t{s}\n')