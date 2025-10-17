#define FILE_NUMBER "P103"

#include "preview.hpp"

class NullPreview : public Preview
{
public:
	NullPreview() : Preview()
	{
	}
	~NullPreview() {}
	virtual void Show(int fd, size_t size, StreamInfo &info) override
	{
		done_callback_(fd);
	}
	void Reset() override {}
	virtual void MaxImageSize(unsigned int &w, unsigned int &h) const override { w = h = 0; }

private:
};

Preview *make_null_preview()
{
	return new NullPreview();
}
