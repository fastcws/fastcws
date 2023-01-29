# fastcws

*轻量级高性能中文分词项目*

## 命令行工具

`fastcws`命令行工具（从源码编译的话，位于`src/tools/fastcws`）可以直接将`stdin`的输入按句分词后输出到`stdout`：

```bash
$ fastcws
在春风吹拂的季节翩翩起舞
在/春风/吹拂/的/季节/翩翩起舞/

```

可以用管道方便的将文件分词后，转储到另一个文件：

```bash
$ cat input.txt | fastcws > output.txt
```

此外，还支持自定义分隔符、从文件加载词典、HMM模型等，详见`fastcws --help`。

### `Windows` 注意事项

在`Windows`平台上，默认的编码是`utf16`，但是本项目目前只使用`utf8`作为唯一编码。

在直接用命令行界面进行输入时，无需考虑此问题，因为工具使用了`nowide`进行自动转换：

```bash
$ fastcws
在春风吹拂的季节翩翩起舞
在/春风/吹拂/的/季节/翩翩起舞/

```

在使用管道分词文件时，必须确认文件以`utf8`格式保存且不带 BOM，否则可能导致分词工作不正常或者出现错误：

```bash
$ type input.txt | fastcws.exe > output.txt
```
必须保证`input.txt`是以`utf8`格式保存的。

## C语言函数库

本项目以c++17写成，不过可以使用编译得到的动态链接库，以稳定的 C 语言 API 调用分词组件：

```c
// #include "libfastcws.h"

fastcws_init();
fastcws_result* result = fastcws_alloc_result();

int err = fastcws_word_break("在春风吹拂的季节翩翩起舞", result);
if (err) {
	...
}
const char *word_begin;
size_t word_len;
while(fastcws_result_next(result, &word_begin, &word_len) == 0) {
	...
}
fastcws_result_free(result);
```

如你所见，分词是0拷贝的，因此性能十分优秀。

此外，C API 同样支持从文件加载词典、HMM模型等。[examples](examples/)目录下有更多范例可供参考。

同样需要注意的是，传入的数据编码必须是`utf8`。

## 编译安装

和多数`cmake`项目一样：

```bash
git submodule update --init --recursive
cmake -S . -B build
cmake --build build
cmake --build build --target install
```

