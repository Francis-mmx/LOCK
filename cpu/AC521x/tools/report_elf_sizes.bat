@rem 放到下载目录中
@rem 会把 sdk.elf 中使用的空间大小汇总
set ELF=main.or32
set OUTPUT=symbol_tbl.html
python report_elf_sizes.py --elf %ELF% --output %OUTPUT%
pause