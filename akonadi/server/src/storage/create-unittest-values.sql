DELETE FROM PimItemFlagRelation;
DELETE FROM PimItemTable;
DELETE FROM PartTable;
DELETE FROM LocationMimeTypeRelation;
DELETE FROM LocationTable WHERE name != "Search" or parentId != 0;
DELETE FROM ResourceTable WHERE name != "akonadi_search_resource";

INSERT INTO ResourceTable (id, name, cachePolicyId) VALUES(2, 'akonadi_dummy_resource_1', 1);
INSERT INTO ResourceTable (id, name, cachePolicyId) VALUES(3, 'akonadi_dummy_resource_2', 1);
INSERT INTO ResourceTable (id, name, cachePolicyId) VALUES(4, 'akonadi_dummy_resource_3', 1);

INSERT INTO LocationTable (id, parentId, name, cachePolicyId, resourceId, existCount, recentCount, unseenCount, firstUnseen) VALUES (10, 6, 'foo', 3, 2, 3, 0, 0, 0);
INSERT INTO LocationTable (id, parentId, name, cachePolicyId, resourceId, existCount, recentCount, unseenCount, firstUnseen) VALUES (2, 10, 'bar', 1, 2, 2, 0, 0, 0);
INSERT INTO LocationTable (id, parentId, name, cachePolicyId, resourceId, existCount, recentCount, unseenCount, firstUnseen) VALUES (3, 2, 'bla', 1, 2, 2, 0, 0, 0);
INSERT INTO LocationTable (id, parentId, name, cachePolicyId, resourceId, existCount, recentCount, unseenCount, firstUnseen) VALUES (4, 10, 'bla', 1, 2, 0, 0, 0, 0);
INSERT INTO LocationTable (id, parentId, name, cachePolicyId, resourceId, existCount, recentCount, unseenCount, firstUnseen) VALUES (5, 7, 'foo2', 1, 3, 2, 3, 4, 5);
INSERT INTO LocationTable (id, parentId, name, cachePolicyId, resourceId, existCount, recentCount, unseenCount, firstUnseen) VALUES (6, 0, 'res1', 1, 2, 0, 0, 0, 0);
INSERT INTO LocationTable (id, parentId, name, cachePolicyId, resourceId, existCount, recentCount, unseenCount, firstUnseen) VALUES (7, 0, 'res2', 1, 3, 0, 0, 0, 0);
INSERT INTO LocationTable (id, parentId, name, cachePolicyId, resourceId, existCount, recentCount, unseenCount, firstUnseen) VALUES (8, 0, 'res3', 1, 4, 0, 0, 0, 0);
INSERT INTO LocationTable (id, parentId, name, cachePolicyId, resourceId, existCount, recentCount, unseenCount, firstUnseen) VALUES (9, 7, 'space folder', 1, 3, 0, 0, 0, 0);

INSERT INTO LocationTable (parentId, name, remoteId, resourceId) VALUES(1, 'kde-core-devel', '<request><userQuery>kde-core-devel@kde.org</userQuery></request>', 1);
INSERT INTO LocationTable (parentId, name, remoteId, resourceId) VALUES(1, 'all', '<request><userQuery>MIMETYPE message/rfc822</userQuery></request>', 1);
INSERT INTO LocationTable (parentId, name, remoteId, resourceId) VALUES(1, 'Test ?er', '<request><userQuery>"Test ?er"</userQuery></request>', 1);

INSERT INTO LocationMimeTypeRelation ( Location_id, MimeType_id) VALUES( 10, 3);
INSERT INTO LocationMimeTypeRelation ( Location_id, MimeType_id) VALUES( 10, 4);
INSERT INTO LocationMimeTypeRelation ( Location_id, MimeType_id) VALUES( 10, 2);
INSERT INTO LocationMimeTypeRelation ( Location_id, MimeType_id) VALUES( 2, 5);
INSERT INTO LocationMimeTypeRelation ( Location_id, MimeType_id) VALUES( 10, 1);
INSERT INTO LocationMimeTypeRelation ( Location_id, MimeType_id) VALUES( 3, 5);
INSERT INTO LocationMimeTypeRelation ( Location_id, MimeType_id) VALUES( 4, 5);
INSERT INTO LocationMimeTypeRelation ( Location_id, MimeType_id) VALUES( 8, 5);
INSERT INTO LocationMimeTypeRelation ( Location_id, MimeType_id) VALUES( 8, 1);

INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (1, 'A', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (2, 'B', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (3, 'C', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (4, 'D', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (5, 'E', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (6, 'F', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (7, 'G', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (8, 'H', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (9, 'I', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (10, 'J', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (11, 'K', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (12, 'L', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (13, 'M', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (14, 'N', 10, 1);
INSERT INTO PimItemTable (id, remoteId, locationId, mimeTypeId) VALUES (15, 'O', 10, 1);

INSERT INTO PimItemFlagRelation (Flag_id, PimItem_id) VALUES (5, 1);
INSERT INTO PimItemFlagRelation (Flag_id, PimItem_id) VALUES (8, 1);
INSERT INTO PimItemFlagRelation (Flag_id, PimItem_id) VALUES (7, 1);
INSERT INTO PimItemFlagRelation (Flag_id, PimItem_id) VALUES (5, 2);

INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (1, 'RFC822', 'testmailbody', 12, 1);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (2, 'HEAD', 'From: <test@user.tst>', 21, 1);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (3, 'RFC822', 'testmailbody1', 13, 2);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (4, 'HEAD', 'From: <test1@user.tst>', 22, 2);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (5, 'RFC822', 'testmailbody2', 13, 3);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (6, 'HEAD', 'From: <test2@user.tst>', 22, 3);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (7, 'RFC822', 'testmailbody3', 13, 4);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (8, 'HEAD', 'From: <test3@user.tst>', 22, 4);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (9, 'RFC822', 'testmailbody4', 13, 5);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (10, 'HEAD', 'From: <test4@user.tst>', 22, 5);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (11, 'RFC822', 'testmailbody5', 13, 6);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (12, 'HEAD', 'From: <test5@user.tst>', 22, 6);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (13, 'RFC822', 'testmailbody6', 13, 7);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (14, 'HEAD', 'From: <test6@user.tst>', 22, 7);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (15, 'RFC822', 'testmailbody7', 13, 8);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (16, 'HEAD', 'From: <test7@user.tst>', 22, 8);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (17, 'RFC822', 'testmailbody8', 13, 9);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (18, 'HEAD', 'From: <test8@user.tst>', 22, 9);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (19, 'RFC822', 'testmailbody9', 13, 10);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (20, 'HEAD', 'From: <test9@user.tst>', 22, 10);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (21, 'RFC822', 'testmailbody10', 14, 11);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (22, 'HEAD', 'From: <test10@user.tst>', 23, 11);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (23, 'RFC822', 'testmailbody11', 14, 12);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (24, 'HEAD', 'From: <test11@user.tst>', 23, 12);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (25, 'RFC822', 'testmailbody12', 14, 13);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (26, 'HEAD', 'From: <test12@user.tst>', 23, 13);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (27, 'RFC822', 'testmailbody13', 14, 14);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (28, 'HEAD', 'From: <test13@user.tst>', 23, 14);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (29, 'RFC822', 'testmailbody14', 14, 15);
INSERT INTO PartTable (id, name, data, datasize, pimItemId) VALUES (30, 'HEAD', 'From: <test14@user.tst>', 23, 15);
