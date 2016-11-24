head
	title => string "Server Info"
body
	table
		tr
			td
				strong => string "Document Root"
			td
				span => param DOCUMENT_ROOT
		tr
			td
				strong => string "Remote Address"
			td
				span => param REMOTE_ADDR
		tr
			td
				strong => string "Request URI"
			td
				span => param REQUEST_URI
		tr
			td
				strong => string "Request Method"
			td
				span => param REQUEST_METHOD
