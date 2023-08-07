#include "CuTest.h"

char json[] = "{'id': 'chatcmpl-7kiJx4kLB3AFaUr4fSyFdkWaK6Op4', 'object': 'chat.completion', 'created': 1691367813, 'model': 'gpt-3.5-turbo-0613', 'choices': [{'index': 0, 'message': {'role': 'assistant', 'content': 'display a[i]'}, 'finish_reason': 'stop'}], 'usage': {'prompt_tokens': 106, 'completion_tokens': 4, 'total_tokens': 110 }}";

void TestJsonParse(CuTest* tc)
{
	CuAssertStrEquals(tc, "display a[i]", "display a[i]");
}

CuSuite* CuJsonSuite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, TestJsonParse);

	return suite;
}


