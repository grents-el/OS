#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/proc_fs.h> 
#include <linux/uaccess.h> 
 
static int K = 0; // Порядок текущего члена ряда Тейлора 
static long long current_sum = 0; // Текущая сумма ряда 
 
// Функция возведения в степень (x^n) 
static long long power(long long x, int n) {  
long long result = 1;
    long long base = x;    
    while (n > 0) {
    if (n % 2 == 1) {
            result *= base;       
            }
        base *= base;       
        n /= 2;
    }    
    return result;}
 
// Функция вычисления факториала (n!) 
static long long factorial(int n) { 
    long long result = 1; 
    for (int i = 1; i <= n; i++) { 
        result *= i; 
    } 
    return result; 
} 
 
static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) { 
    static char buffer[256]; 
    static int finished = 0; 
 
    if (finished) { 
        finished = 0; 
        return 0; 
    } 
 
    long long x = 78; // Значение x для ряда 
    long long term;    // Текущий член ряда 
    long long sign = (K % 2 == 0) ? 1 : -1; // Знак текущего члена ряда 
 
    // Вычисление текущего члена ряда 
    term = sign * power(x, 2 * K) / factorial(2 * K); 
 
    // Обновляем сумму 
    current_sum += term; 
 
    // Форматируем сообщение для вывода 
    int len = snprintf(buffer, sizeof(buffer), 
                       "Член ряда Тейлора с порядком %d: %lld (x^%d / %d!)\n", 
                       K, term, 2 * K, 2 * K); 
    len += snprintf(buffer + len, sizeof(buffer) - len, 
                    "Текущая сумма: %lld\n", current_sum); 
 
    if (copy_to_user(buf, buffer, len)) { 
        return -EFAULT; 
    } 
 
    K++; // Увеличиваем порядок 
    finished = 1; 
    return len; 
} 
 
static struct proc_dir_entry *proc_file; 
static const struct proc_ops proc_file_ops = { 
    .proc_read = proc_read, 
}; 
 
static int __init my_init(void) { 
    proc_file = proc_create("tsulab", 0444, NULL, &proc_file_ops); 
    if (!proc_file) { 
        pr_err("Не удалось создать файл в /proc\n"); 
        return -ENOMEM; 
    } 
 
    pr_info("Модуль загружен: файл %s создан в /proc\n", "tsulab"); 
    return 0; 
} 
 
static void __exit my_exit(void) { 
    if (proc_file) { 
        proc_remove(proc_file); 
        pr_info("Файл %s удален из /proc\n", "tsulab"); 
    } 
 
    pr_info("Tomsk State University forever!\n"); 
} 
 
module_init(my_init); 
module_exit(my_exit); 
 
MODULE_LICENSE("GPL");
