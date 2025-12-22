import random
import sys

# Имена файлов для вывода
DATA_FILE = "numbers.txt"
AVERAGE_FILE = "average_result.txt"
BITS = 512

def generate_and_calculate():
    num_count_str = input(f"Введите количество {BITS}-битных чисел для генерации (например, 1000): ")
    num_count = int(num_count_str)
    if num_count <= 0:
        print("Ошибка: Количество чисел должно быть положительным.")
        return

    total_sum = 0
    
    print(f"\nНачало генерации и записи {num_count} чисел в файл '{DATA_FILE}'...")

    with open(DATA_FILE, 'w') as f:
        f.write(f"{num_count}\n")
        
        for _ in range(num_count):
            random_512_bit_int = random.getrandbits(BITS)
            total_sum += random_512_bit_int
            hex_str = hex(random_512_bit_int)
            
            f.write(f"{hex_str}\n")
    
    rounded_average_value = (total_sum + num_count // 2) // num_count
        
    print(f"Запись среднего арифметического в файл '{AVERAGE_FILE}'...")
    
    with open(AVERAGE_FILE, 'w') as f:
        f.write("--- Среднее Арифметическое (Округлено до целого) ---\n")
        f.write(f"Количество чисел (N): {num_count}\n")
        f.write(f"Общая сумма (Sum): {total_sum}\n\n")
        
        # Результат в HEX
        hex_average = hex(rounded_average_value)
        f.write(f"HEX: {hex_average}\n")
        
        # Результат в DEC
        f.write(f"DEC: {rounded_average_value}\n")

    print("\n✅ Успешно завершено!")
    print(f"   - Сгенерировано: {num_count} чисел.")
    print(f"   - Данные записаны в: {DATA_FILE}")
    print(f"   - Результат записан в: {AVERAGE_FILE}")

if __name__ == "__main__":
    generate_and_calculate()